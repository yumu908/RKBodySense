//
// Created by Optical on 2024/3/25.
//

#include "image_reader.h"
#include "linux/videodev2.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <bits/ioctl.h>

#include <android/log.h>
#include <string.h>
#include <malloc.h>

#define TAG "ImageReader"

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define NV_MEMSET(x) memset(&(x), 0, sizeof(x))

#define BUILD_LIBRGA 1
#define BUILD_PRINTF 1
#define BUFFER_COUNT 4
#define MAX_PLANES   2 // NV12 有两个平面：Y 和 UV

#define V4L2_MAX_BUF_NUM 3
typedef struct _v4l2_context {
    // user parameters
    uint32_t width;
    uint32_t height;
    uint32_t format;

    // runtime parameters
    uint32_t fd;
    uint32_t  plane_count;
    uint32_t  buffer_count;
    uint32_t  buffer_size;
    void*     buffers[BUFFER_COUNT];
    int       buffer_fds[BUFFER_COUNT];
    char  dev_name[64];
} v4l2_context;

const char* v4l2_format_to_string(__u32 pixelformat) {
    // 只列出一些常见的格式，可以根据需要添加更多
    switch (pixelformat) {
        case V4L2_PIX_FMT_YUYV:   return "YUYV";
        case V4L2_PIX_FMT_UYVY:   return "UYVY";
        case V4L2_PIX_FMT_YVYU:   return "YVYU";
        case V4L2_PIX_FMT_VYUY:   return "VYUY";
        case V4L2_PIX_FMT_RGB24:  return "RGB24";
        case V4L2_PIX_FMT_BGR24:  return "BGR24";
        case V4L2_PIX_FMT_RGB32:  return "RGB32";
        case V4L2_PIX_FMT_BGR32:  return "BGR32";
        case V4L2_PIX_FMT_MJPEG:  return "MJPEG";
        case V4L2_PIX_FMT_JPEG:   return "JPEG";
        case V4L2_PIX_FMT_H264:   return "H264";
        case V4L2_PIX_FMT_NV12:   return "NV12";
        case V4L2_PIX_FMT_NV21:   return "NV21";
        case V4L2_PIX_FMT_GREY:   return "GREY";
        case V4L2_PIX_FMT_RGB565: return "RGB565";
        default:
            // 对于未知格式，尝试将其解释为四字符代码 (FourCC)
            static char fourcc[5];
            fourcc[0] = (char)(pixelformat & 0xFF);
            fourcc[1] = (char)((pixelformat >> 8) & 0xFF);
            fourcc[2] = (char)((pixelformat >> 16) & 0xFF);
            fourcc[3] = (char)((pixelformat >> 24) & 0xFF);
            fourcc[4] = '\0';
            return fourcc;
    }
}

float calculate_fps(__u32 numerator, __u32 denominator) {
    if (denominator == 0) return 0.0f; // 防止除零
    return (float)denominator / (float)numerator;
}

void v4l2_query_frame_intervals(int fd, __u32 pixelformat, __u32 width, __u32 height) {
    struct v4l2_frmivalenum fival;
    memset(&fival, 0, sizeof(fival));
    fival.pixel_format = pixelformat;
    fival.width = width;
    fival.height = height;
    fival.index = 0;

    printf("      Frame Intervals:\n");
    while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &fival) == 0) {
        // 根据类型打印帧率
        if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
            float fps = calculate_fps(fival.discrete.numerator, fival.discrete.denominator);
            ALOGD("        %d/%d (%.2f FPS)",
                   fival.discrete.numerator, fival.discrete.denominator, fps);
        } else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
            float min_fps = calculate_fps(fival.stepwise.max.numerator, fival.stepwise.max.denominator);
            float max_fps = calculate_fps(fival.stepwise.min.numerator, fival.stepwise.min.denominator);
            ALOGD("        Stepwise: min=%d/%d (%.2f FPS), max=%d/%d (%.2f FPS), step=%d/%d",
                   fival.stepwise.min.numerator, fival.stepwise.min.denominator, min_fps,
                   fival.stepwise.max.numerator, fival.stepwise.max.denominator, max_fps,
                   fival.stepwise.step.numerator, fival.stepwise.step.denominator);
            break;
        } else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
            float min_fps = calculate_fps(fival.stepwise.max.numerator, fival.stepwise.max.denominator);
            float max_fps = calculate_fps(fival.stepwise.min.numerator, fival.stepwise.min.denominator);
            ALOGD("        Continuous: %.2f FPS to %.2f FPS", min_fps, max_fps);
            break;
        }
        fival.index++;
    }
    // 如果没有支持的帧率或查询失败，可能不打印或打印错误信息
    if (fival.index == 0) {
        printf("        (No frame intervals reported or query failed)");
    }
}

void v4l2_query_frame_sizes(int fd, __u32 pixelformat) {
    struct v4l2_frmsizeenum fsize;
    memset(&fsize, 0, sizeof(fsize));
    fsize.pixel_format = pixelformat;
    fsize.index = 0;

    while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &fsize) == 0) {
        if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            ALOGD("      %dx%d\n", fsize.discrete.width, fsize.discrete.height);
            // 对于离散尺寸，查询支持的帧率
            v4l2_query_frame_intervals(fd, pixelformat, fsize.discrete.width, fsize.discrete.height);
        } else if (fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
            ALOGD("      Stepwise: min=%dx%d, max=%dx%d, step=%dx%d\n",
                   fsize.stepwise.min_width, fsize.stepwise.min_height,
                   fsize.stepwise.max_width, fsize.stepwise.max_height,
                   fsize.stepwise.step_width, fsize.stepwise.step_height);
            break;
        } else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
            ALOGD("      Continuous: min=%dx%d, max=%dx%d\n",
                   fsize.stepwise.min_width, fsize.stepwise.min_height,
                   fsize.stepwise.max_width, fsize.stepwise.max_height);
            break; // Continuous 通常只返回一个条目描述范围
        }
        fsize.index++;
    }
}

void v4l2_enum_video_format(int fd, struct v4l2_capability *capacity) {
    struct v4l2_fmtdesc fmt = {0};
    memset(&fmt, 0, sizeof(fmt));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    ALOGD("\nSupported Formats and Frame Sizes:");
    ALOGD("==================================");

    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0) {
        const char *format_str = v4l2_format_to_string(fmt.pixelformat);
        ALOGD("Format %u: %s ('%c%c%c%c')\n", fmt.index, format_str,
               (fmt.pixelformat & 0xFF), ((fmt.pixelformat >> 8) & 0xFF),
               ((fmt.pixelformat >> 16) & 0xFF), ((fmt.pixelformat >> 24) & 0xFF));
        v4l2_query_frame_sizes(fd, fmt.pixelformat);
        fmt.index++;
    }
}

int v4l2_video_query_capacity(v4l2_context* ctx) {
    if ((nullptr == ctx) || (ctx->fd <= 0)) {
        ALOGE("invalid parameters");
        return -1;
    }
    // request and check v4l2 capability
    struct v4l2_capability capacity = {0};
    if (ioctl(ctx->fd, VIDIOC_QUERYCAP, &capacity) < 0) {
        ALOGE("failed to ioctl(%d, VIDIOC_QUERYCAP, &capacity)", ctx->fd);
        return -1;
    } else {
        ALOGD("Capacity Information:");
        ALOGD("\tDriver name:       %s", capacity.driver);
        ALOGD("\tCard type:         %s", capacity.card);
        ALOGD("\tBus info:          %s", capacity.bus_info);
        ALOGD("\tDriver version:    %u.%u.%u", (capacity.version >>16) & 0xFF,
              (capacity.version >>8) & 0xFF, capacity.version & 0xFF);
        ALOGD("\tCapabilities:      0x%08X", capacity.capabilities);
        ALOGD("\tDevice caps:       0x%08X", capacity.device_caps);
    }
    if (!(capacity.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        ALOGE("%s does not support V4L2_CAP_VIDEO_CAPTURE_MPLANE", ctx->dev_name);
        return -1;
    } else {
        v4l2_enum_video_format(ctx->fd, &capacity);
    }

    if (!(capacity.capabilities & V4L2_CAP_STREAMING)) {
        ALOGE("%s does not support V4L2_CAP_STREAMING", ctx->dev_name);
    }
    return 0;
}

int v4l2_video_setformat(v4l2_context* ctx, int width, int height, int type) {
    if ((nullptr == ctx) || (ctx->fd <= 0)) {
        ALOGE("invalid parameters");
        return -1;
    }

    int video_fd = ctx->fd;
    struct v4l2_format format = {0};
    NV_MEMSET(format);
    format.type = type;
    format.fmt.pix.width  = width;
    format.fmt.pix.height = height;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
    if (ioctl(video_fd, VIDIOC_S_FMT, &format) < 0) {
        ALOGE("failed to ioctl(%d, VIDIOC_S_FMT, &format)", video_fd);
        return -1;
    } else {
        ALOGE("succeeded to ioctl(%d, VIDIOC_S_FMT, &format)", video_fd);
        ALOGE("video format: %dx%d - V4L2_PIX_FMT_NV12", width, height);
    }
    // 确定平面数量
    ctx->plane_count = format.fmt.pix_mp.num_planes;
    ALOGD("plane_count=%d, MAX_PLANES=%d", ctx->plane_count, MAX_PLANES);
    return 0;
}

int v4l2_video_query_buffers(v4l2_context* ctx, int type) {
    if ((nullptr == ctx) || (ctx->fd <= 0)) {
        ALOGE("invalid parameters");
        return -1;
    }

    int video_fd = ctx->fd;
    int ret = 0;

    // Step 1: request buffers
    struct v4l2_requestbuffers req_buffers = {0};
    NV_MEMSET(req_buffers);
    req_buffers.count  = BUFFER_COUNT;
    req_buffers.memory = V4L2_MEMORY_MMAP;
    req_buffers.type   = type;
    if (ioctl(video_fd, VIDIOC_REQBUFS, &req_buffers) < 0) {
        ALOGE("failed to ioctl(%d, VIDIOC_REQBUFS, &req_buffers)", video_fd);
        return -1;
    }

    // Check if the driver granted the requested number of buffers
    if (req_buffers.count < BUFFER_COUNT) {
        ALOGE("Not enough buffers granted. Got %d, but wanted %d.", req_buffers.count, BUFFER_COUNT);
        return -1;
    }
    ctx->buffer_count = req_buffers.count;
    ALOGD("succeeded to request buffers %d buffers granted.", ctx->buffer_count);

    // Step 2-5: Query, map, enqueue, and export each buffer
    for (int idx = 0; idx < req_buffers.count; idx++) {
        struct v4l2_buffer buffer = {0};
        struct v4l2_plane planes[MAX_PLANES];
        NV_MEMSET(buffer);
        buffer.index = idx;
        buffer.type = type;
        buffer.memory = V4L2_MEMORY_MMAP;
        // buffer.flags  = V4L2_BUF_FLAG_NO_CACHE_INVALIDATE | V4L2_BUF_FLAG_NO_CACHE_CLEAN;
        buffer.m.planes = planes;
        buffer.length   = ctx->plane_count;

        // Query buffer information
        if (ioctl(video_fd, VIDIOC_QUERYBUF, &buffer) < 0) {
            ALOGE("failed to ioctl(%d, VIDIOC_QUERYBUF, &buffer) index=%d", video_fd, idx);
            return -1;
        } else {
            ALOGE("succeeded to ioctl(%d, VIDIOC_QUERYBUF, &buffer) index=%d", video_fd, idx);
        }

        // Map the buffer into the user-space
        ctx->buffers[idx] = mmap(nullptr, buffer.m.planes->length, \
                PROT_READ | PROT_WRITE, MAP_SHARED, video_fd, buffer.m.planes->m.mem_offset);
        if (ctx->buffers[idx] == MAP_FAILED) {
            ALOGE("failed to mmap(... fd=%d) index=%d error:%s", video_fd, idx, strerror(errno));
            return -1;
        } else {
            ALOGE("succeeded to mmap(... ) buffer=%p size=%d", ctx->buffers[idx], buffer.m.planes->length);
        }

        // Store buffer size on the first iteration
        if (idx == 0) {
            ctx->buffer_size = buffer.m.planes->length;
        }

        // Enqueue the buffer
        if (ioctl(video_fd, VIDIOC_QBUF, &buffer) < 0) {
            ALOGD("failed to ioctl(%d, VIDIOC_QBUF, &buffer). error=%s", video_fd, strerror(errno));
            return -1;
        }

        // Export a file descriptor for the buffer
        struct v4l2_exportbuffer exp_buf = {0};
        exp_buf.type  = buffer.type;
        exp_buf.index = buffer.index;
        if (ioctl(video_fd, VIDIOC_EXPBUF, &exp_buf) < 0) {
            ALOGD("failed to ioctl(%d, VIDIOC_EXPBUF, &exp_buf). error=%s", video_fd, strerror(errno));
            return -1;
        } else {
            ctx->buffer_fds[idx] = exp_buf.fd;
        }
    }

    return 0;
}

int v4l2_video_setfps(v4l2_context* ctx, int type) {
    if ((nullptr == ctx) || (ctx->fd <= 0)) {
        ALOGE("invalid parameters");
        return -1;
    }

    // config new frame-rate(e.g. 60FPS)
    // frame-rate = denominator / numerator
    // 60 FPS = 60/1
    int video_fd = ctx->fd;
    struct v4l2_streamparm vs_param = {0};
    NV_MEMSET(vs_param);
    vs_param.type = type;
    vs_param.parm.capture.timeperframe.numerator   = 1;
    vs_param.parm.capture.timeperframe.denominator = 60;
    if(ioctl(video_fd, VIDIOC_S_PARM, &vs_param) < 0) {
        ALOGD("failed to ioctl(%d, VIDIOC_S_PARM, &vs_param). error=%s", video_fd, strerror(errno));
    } else {
        ALOGE("successed to setfps(60)");
    }

    // query video fps
    NV_MEMSET(vs_param);
    vs_param.type = type;
    if(ioctl(video_fd, VIDIOC_G_PARM, &vs_param) < 0) {
        ALOGD("failed to ioctl(%d, VIDIOC_G_PARM, &vs_param", video_fd);
    } else {
        ALOGD("successed to getfps() = %d FPS", vs_param.parm.capture.timeperframe.denominator);
    }
    return 0;
}

void* v4l2_video_release(v4l2_context* ctx) {
    if (nullptr != ctx) {
        if (ctx->fd > 0) {
            close(ctx->fd);
            ctx->fd = 0;
        }
        free(ctx);
    }
    return nullptr;
}

// uri = "/dev/video12"
void* v4l2_reader_open(const char* uri, int width, int height) {
    ALOGD("try to open(%s) w=%d h=%d fmt=%d", uri, width, height, V4L2_PIX_FMT_NV12);
    int video_fd = open(uri, O_RDWR);
    if (video_fd < 0) {
        ALOGE("failed to open(%s, O_RDWR)", uri);
        return nullptr;
    }

    // malloc runtime parameters
    auto *ctx = (v4l2_context*)malloc(sizeof(v4l2_context));
    ctx->fd     = video_fd;
    ctx->width  = width;
    ctx->height = height;
    ctx->format = V4L2_PIX_FMT_NV12;
    sprintf(ctx->dev_name, "%s", uri);

    // best buf_type for video_capture
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    // query and check video capacity
    if (v4l2_video_query_capacity(ctx) < 0) {
        return v4l2_video_release(ctx);
    }
    // query and check video format
    if (v4l2_video_setformat(ctx, width, height, type) < 0) {
        return v4l2_video_release(ctx);
    }
    // query and check video fps before VIDIOC_QBUF/VIDIOC_STREAMON
    if (v4l2_video_setfps(ctx, type) < 0) {
        return v4l2_video_release(ctx);
    }
    // query and check video buffers
    if (v4l2_video_query_buffers(ctx, type) < 0) {
        return v4l2_video_release(ctx);
    }

    // start camera stream
    if (ioctl(video_fd, VIDIOC_STREAMON, &type) < 0) {
        ALOGD("failed to ioctl(%d, VIDIOC_STREAMON, ...) error:%s", video_fd, strerror(errno));
        return v4l2_video_release(ctx);
    } else {
        ALOGE("succeeded to ioctl(%d, VIDIOC_STREAMON, ...)", video_fd);
    }

    return ctx;
}

int v4l2_reader_close(void* context) {
    auto *ctx = (v4l2_context*)context;
    if ((nullptr == ctx) || (ctx->fd <= 0)) {
        ALOGE("invalid parameters");
        return -1;
    }

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (ioctl(ctx->fd, VIDIOC_STREAMOFF, &type) < 0) {
        ALOGE("failed to ioctl(%d, VIDIOC_STREAMOFF, ...)", ctx->fd);
    } else {
        ALOGE("succeeded to ioctl(%d, VIDIOC_STREAMOFF, ...)", ctx->fd);
    }

    // unmap v4l2-buffers
    munmap(ctx->buffers, ctx->buffer_count * ctx->buffer_size);
    close(ctx->fd);
    ctx->fd = 0;

    return 0;
}

int v4l2_reader_dequeue(void* context, NVImage* image) {
    auto *ctx = (v4l2_context*)context;
    if ((nullptr == ctx) || (ctx->fd <= 0)) {
        ALOGE("invalid parameters");
        return -1;
    }
    memset(image, 0, sizeof(NVImage));

    // prepare the v4l2_buffer structure
    struct v4l2_buffer buffer = {0};
    struct v4l2_plane  planes[MAX_PLANES];
    buffer.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.m.planes = planes;
    buffer.length   = ctx->plane_count;

    // wait and dequeue ready v4l2 buffer
    if (ioctl(ctx->fd, VIDIOC_DQBUF, &buffer) < 0) {
        if (errno == EAGAIN) {
            // This is not an error, just no frame available yet.
            // You might need to use select() or poll() before DQBUF.
            ALOGD("no frame available, try again later.");
            return 0;
        }
        ALOGE("failed to ioctl(%d, VIDIOC_DQBUF, &buffer) error=%s", ctx->fd, strerror(errno));
        return -1;
    }

    // copy v4l2 buffer to RockIvaImage
    image->buffer.size = buffer.m.planes->length;
    image->buffer.virt_addr = (uint8_t*)ctx->buffers[buffer.index];
    image->buffer.phys_addr = nullptr;
    image->width  = ctx->width;
    image->height = ctx->height;
    image->format = ctx->format;

    // ALOGD("dequeue buffer[%d]: %p %dx%d", buffer.index, image->buffer.virt_addr, image->width, image->height);

    return buffer.index;
}

int v4l2_reader_enqueue(void *context, NVImage* image, int index) {
    auto *ctx = (v4l2_context*)context;
    if ((nullptr == ctx) || (ctx->fd <= 0)) {
        ALOGE("invalid parameters");
        return -1;
    }

    // prepare the v4l2_buffer structure
    struct v4l2_buffer buffer = {0};
    struct v4l2_plane  planes[MAX_PLANES];
    buffer.index  = index;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.m.planes = planes;
    buffer.length   = ctx->plane_count;

    // enqueue v4l2 buffer
    if (ioctl(ctx->fd, VIDIOC_QBUF, &buffer) < 0) {
        ALOGE("failed to ioctl(%d, VIDIOC_QBUF, &buffer) error:%s", ctx->fd, strerror(errno));
        return -1;
    }

    // ALOGD("enqueue buffer[%d]: %p %dx%d", buffer.index, image->buffer.virt_addr, image->width, image->height);

    return 0;
}

int v4l2_reader_dump_format(void* ctx) {
    v4l2_context* context = (v4l2_context*)ctx;
    if (nullptr == context) {
        return -1;
    }

    ALOGE("V4L2: dump_format()");
    ALOGE("V4L2: width=%d, height=%d, format=%d", context->width, context->height, context->format);
    ALOGE("V4L2: video=%s buffers(count=%d, size=%d)", context->dev_name, context->buffer_count, context->buffer_size);
    ALOGE("==================\n");

    return 0;
}

// Define V4L2 implementation operation function table
const NVImageReaderOps v4l2_reader_ops = {
    .open    = v4l2_reader_open,
    .close   = v4l2_reader_close,
    .dequeue = v4l2_reader_dequeue,
    .enqueue = v4l2_reader_enqueue,
    .dump_format = v4l2_reader_dump_format
};

// Define V4L2 implementation operation function table
const NVImageReaderOps image_reader_ops = {
   .open    = v4l2_reader_open,
   .close   = v4l2_reader_close,
   .dequeue = v4l2_reader_dequeue,
   .enqueue = v4l2_reader_enqueue,
   .dump_format = v4l2_reader_dump_format
};

NVImageReader* nv_image_reader_reader(NVImageReaderType readerType) {
    NVImageReader* reader = (NVImageReader*)malloc(sizeof(NVImageReader));
    if (nullptr == reader) {
        ALOGE("failed to malloc(...) for NVImageReader");
        return nullptr;
    }
    reader->context = nullptr;

    switch (readerType) {
      case NV_READER_TYPE_IMAGE:
        reader->ops = &image_reader_ops;
        break;
      case NV_READER_TYPE_V4L2:
        reader->ops = &v4l2_reader_ops;
        break;
      default:
        break;
    }
    return reader;
}