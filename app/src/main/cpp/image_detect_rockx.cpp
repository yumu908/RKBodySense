//
// Created by Optical on 2025/3/21.
//

#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <deque>

#include "rockx.h"
#include "rockx_module_type.h"
#include "utils/rockx_data_utils.h"
#include "cJson.h"
#include "image_reader.h"
#include "image_detect_rockx.h"

#define TAG "ROCKX"

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

typedef struct _rockx_task_context {
    // task threads
    pthread_t camera_thread;
    pthread_t rockx_thread;
    pthread_t listen_thread;
    pthread_mutex_t buffer_mutex;
    pthread_mutex_t queue_mutex;
    pthread_cond_t  buffer_ready;

    // task performance
    int32_t    task_run;
    NVPerfInfo perf_read;
    NVPerfInfo perf_rknn;

    // camera module
    NVImageReader* reader;
    std::deque<NVImage> buffers;
    std::deque<char*> results;

    TaskListener onListener;

    // rockx module
    RockXHandle xhandle;
} rockx_task_context;

static rockx_task_context gTaskCtx;
void* thread_worker_reader(void* arg);
void* thread_worker_inference(void* arg);
void* thread_worker_listener(void* arg);

// max_priority = 99  SCHED_FIFO = 0
RockXRetCode bind_thread_to_cpu(pthread_t pid, int cpu_id, int priority) {
    cpu_set_t cpuset = {0};
    struct sched_param param = {0};
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);   // select CPU-3
    sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
    memset(&param, 0, sizeof(struct sched_param));
    param.sched_priority = sched_get_priority_max(priority);
    pthread_setschedparam(pid, priority, &param);

    return ROCKX_RET_SUCCESS;
}

RockXRetCode rockx_task_init(const char* module_name) {
    RockXRetCode ret;
    gTaskCtx.xhandle = RockXCreate(0, nullptr);
    ret = RockXSetParamString(gTaskCtx.xhandle, nullptr, ROCKX_KEY_MODULE_NAME, "PoseX", 0);
    ret = RockXSetParamString(gTaskCtx.xhandle, nullptr, ROCKX_KEY_DATA_PATH, "cust", 0);
    ret = RockXSetParamString(gTaskCtx.xhandle, nullptr, ROCKX_KEY_LICENSE_PATH, "/cust/rockx.lic", 0);
    ret = RockXSetParamInt(gTaskCtx.xhandle, "PoseX.rknn", "core_mask", 3);
    // set rknn model name
    ret = RockXSetParamString(gTaskCtx.xhandle, nullptr, "model_name", "pose_body_v4", 0);

    // init
    ret = RockXInit(gTaskCtx.xhandle);
    if (ret != ROCKX_RET_SUCCESS) {
        ALOGE("Failed to RockXInit. ret=%d", ret);
        return ret;
    }

    pthread_mutex_init(&gTaskCtx.buffer_mutex, nullptr);
    pthread_mutex_init(&gTaskCtx.queue_mutex, nullptr);
    pthread_cond_init(&gTaskCtx.buffer_ready, nullptr);

    // create camera_thread
    memset(&(gTaskCtx.perf_read), 0, sizeof(NVPerfInfo));
    gTaskCtx.perf_read.stat_time = nv_time_ms();
    gTaskCtx.buffers.clear();
    gTaskCtx.reader  = nv_image_reader_reader(NV_READER_TYPE_V4L2);
    nv_image_reader_open(gTaskCtx.reader, "/dev/video12", 640, 360);
    if (pthread_create(&gTaskCtx.camera_thread, NULL, thread_worker_reader, NULL) != 0) {
        ALOGE("Failed to pthread_create for camera_thread");
        gTaskCtx.camera_thread = 0;
    }

    // create rockx_thread
    memset(&(gTaskCtx.perf_rknn), 0, sizeof(NVPerfInfo));
    gTaskCtx.perf_rknn.stat_time = nv_time_ms();
    gTaskCtx.onListener = nullptr;
    gTaskCtx.results.clear();
    if (pthread_create(&gTaskCtx.rockx_thread, NULL, thread_worker_inference, NULL) != 0) {
        ALOGE("Failed to pthread_create for rockx_thread");
        gTaskCtx.rockx_thread = 0;
    }
    if (pthread_create(&gTaskCtx.listen_thread, NULL, thread_worker_listener, NULL) != 0) {
        ALOGE("Failed to pthread_create for listen_thread");
        gTaskCtx.listen_thread = 0;
    }
  #if BUILD_BIND_CPU
    bind_thread_to_cpu(gTaskCtx.camera_thread, 2, 50);
    bind_thread_to_cpu(gTaskCtx.camera_thread, 3, 50);
  #endif

    return ROCKX_RET_SUCCESS;
}

RockXRetCode rockx_task_set_task_listener(TaskListener listener) {
    gTaskCtx.onListener = listener;
    return ROCKX_RET_SUCCESS;
}

void* thread_worker_reader(void* arg) {
    gTaskCtx.task_run = 1;
    while(gTaskCtx.task_run > 0) {
        // read image from reader
        int64_t time_now = nv_time_ms();
        NVImage image = {0};
        int index = nv_image_reader_dequeue(gTaskCtx.reader, &image);
        if (index >= 0) {
            pthread_mutex_lock(&gTaskCtx.buffer_mutex);
            while (gTaskCtx.buffers.size() >= 2) {
                // release used buffers.
                NVImage img_old = gTaskCtx.buffers.back();
                nv_image_reader_enqueue(gTaskCtx.reader, &img_old, img_old.index);
                gTaskCtx.buffers.pop_back();
            }
            image.index     = index;
            image.timestamp = nv_time_ms();
            gTaskCtx.buffers.push_front(image);
            pthread_cond_signal(&gTaskCtx.buffer_ready);
            pthread_mutex_unlock(&gTaskCtx.buffer_mutex);
        }

        // task performance
        gTaskCtx.perf_read.task_time += (nv_time_ms() - time_now);
        gTaskCtx.perf_read.task_num++;
        if ((nv_time_ms() - gTaskCtx.perf_read.stat_time) > 1000) {
            gTaskCtx.perf_read.stat_time = nv_time_ms();
            ALOGD("reader performance: fps=%d, time=%ldms/s", \
                          gTaskCtx.perf_read.task_num, gTaskCtx.perf_read.task_time / gTaskCtx.perf_read.task_num);
            gTaskCtx.perf_read.task_time = 0;
            gTaskCtx.perf_read.task_num  = 0;
        }
    }
    return nullptr;
}

void* thread_worker_inference(void* arg) {
    gTaskCtx.task_run = 1;
    while(gTaskCtx.task_run > 0) {
        char *bodypose = nullptr;
        int64_t time_now = nv_time_ms();
        rockx_task_process(nullptr, &bodypose);
        if (nullptr != bodypose) {
            pthread_mutex_lock(&gTaskCtx.queue_mutex);
            gTaskCtx.results.push_back(bodypose);
            pthread_mutex_unlock(&gTaskCtx.queue_mutex);
        }

        // task performance
        gTaskCtx.perf_rknn.task_time += (nv_time_ms() - time_now);
        gTaskCtx.perf_rknn.task_num++;
        if ((nv_time_ms() - gTaskCtx.perf_rknn.stat_time) > 1000) {
            gTaskCtx.perf_rknn.stat_time = nv_time_ms();
            ALOGD("body-pose performance: fps=%d, time=%ldms/s", \
                          gTaskCtx.perf_rknn.task_num, gTaskCtx.perf_rknn.task_time / gTaskCtx.perf_rknn.task_num);
            gTaskCtx.perf_rknn.task_time = 0;
            gTaskCtx.perf_rknn.task_num  = 0;
        }
    }
    return nullptr;
}

void* thread_worker_listener(void* arg) {
    gTaskCtx.task_run = 1;
    while (gTaskCtx.task_run > 0) {
        pthread_mutex_lock(&gTaskCtx.queue_mutex);
        if (gTaskCtx.results.size() > 0) {
            char *bodypose = gTaskCtx.results.front();
            gTaskCtx.results.pop_front();
            pthread_mutex_unlock(&gTaskCtx.queue_mutex);
            if (nullptr != gTaskCtx.onListener) {
                gTaskCtx.onListener(bodypose, nullptr);
            }
            if (nullptr != bodypose) {
                free(bodypose);
            }
        } else {
            pthread_mutex_unlock(&gTaskCtx.queue_mutex);
            usleep(5*1000);
        }
    }
    return nullptr;
}

RockXRetCode rockx_task_reset() {
    if (nullptr != gTaskCtx.xhandle) {
        RockXDestroy(gTaskCtx.xhandle);
    }
    return ROCKX_RET_SUCCESS;
}

float rockx_task_avg_scores(const RockXBodyLandmark* obj) {
    if (obj->count == 0) return 0.0f;

    float total = 0.0f;
    for (int j = 0; j < obj->count; j++) {
        total += obj->scores[j];
    }
    return total / obj->count;
}

// 将 RockXPoint 转换为 cJSON 数组
cJSON* RockXPointToJSON(RockXPoint point) {
    cJSON *array = cJSON_CreateArray();
    cJSON_AddItemToArray(array, cJSON_CreateNumber(point.x));
    cJSON_AddItemToArray(array, cJSON_CreateNumber(point.y));
    return array;
}

// 将 RockXRect 转换为 cJSON 对象
cJSON* RockXRectToJSON(RockXRect rect) {
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddItemToObject(obj, "left", cJSON_CreateNumber(rect.left));
    cJSON_AddItemToObject(obj, "top", cJSON_CreateNumber(rect.top));
    cJSON_AddItemToObject(obj, "right", cJSON_CreateNumber(rect.right));
    cJSON_AddItemToObject(obj, "bottom", cJSON_CreateNumber(rect.bottom));
    return obj;
}

// 将 RockXRectRotated 转换为 cJSON 对象
cJSON* RockXRectRotatedToJSON(RockXRectRotated rect) {
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddItemToObject(obj, "cx", cJSON_CreateNumber(rect.centerx));
    cJSON_AddItemToObject(obj, "cy", cJSON_CreateNumber(rect.centery));
    cJSON_AddItemToObject(obj, "width", cJSON_CreateNumber(rect.width));
    cJSON_AddItemToObject(obj, "height", cJSON_CreateNumber(rect.height));
    cJSON_AddItemToObject(obj, "angle", cJSON_CreateNumber(rect.angle));
    return obj;
}

// 将 RockXBodyLandmark 转换为 cJSON 对象
cJSON* RockXBodyLandmarkToJSON(RockXBodyLandmark* landmark) {
    cJSON *obj = cJSON_CreateObject();

    cJSON_AddItemToObject(obj, "id",    cJSON_CreateNumber(landmark->id));
    cJSON_AddItemToObject(obj, "score", cJSON_CreateNumber(landmark->score));
    cJSON_AddItemToObject(obj, "box",   RockXRectToJSON(landmark->box));
    cJSON_AddItemToObject(obj, "leftHandBox",    RockXRectRotatedToJSON(landmark->leftHandBox));
    cJSON_AddItemToObject(obj, "rightHandBox",   RockXRectRotatedToJSON(landmark->rightHandBox));
    cJSON_AddItemToObject(obj, "leftHandScore",  cJSON_CreateNumber(landmark->leftHandScore));
    cJSON_AddItemToObject(obj, "rightHandScore", cJSON_CreateNumber(landmark->rightHandScore));
    cJSON_AddItemToObject(obj, "count", cJSON_CreateNumber(landmark->count));

    // 添加关键点数组
    cJSON *pointsArray = cJSON_CreateArray();
    for (int i = 0; i < landmark->count; i++) {
        cJSON_AddItemToArray(pointsArray, RockXPointToJSON(landmark->points[i]));
    }
    cJSON_AddItemToObject(obj, "points", pointsArray);

    // 添加关键点置信度数组
    cJSON *scoresArray = cJSON_CreateArray();
    for (int i = 0; i < landmark->count; i++) {
        cJSON_AddItemToArray(scoresArray, cJSON_CreateNumber(landmark->scores[i]));
    }
    cJSON_AddItemToObject(obj, "scores", scoresArray);

    return obj;
}

RockXRetCode rockx_task_process(RockXImage *image, char** json_objects) {
    RockXRetCode ret = ROCKX_RET_SUCCESS;
    if (nullptr == gTaskCtx.xhandle) {
        ALOGE("invalid handle, body-pose model is not initialized?");
        return ret;
    }
    RockXInput* input = RockXInputCreate(gTaskCtx.xhandle);
    if (input == nullptr) {
        ALOGE("Failed to RockXInputCreate");
        return ret;
    }

    int index = 0;
    NVImage    nv_image = {0};
    RockXImage rx_image = {0};
    input->packets[0].type  = ROCKX_PACKET_TYPE_IMAGE;
    if (nullptr != image) {
        input->packets[0].image = *image;
    } else {
        pthread_mutex_lock(&gTaskCtx.buffer_mutex);
        if (gTaskCtx.buffers.empty()) {
            pthread_cond_wait(&gTaskCtx.buffer_ready, &gTaskCtx.buffer_mutex);
        }
        nv_image = gTaskCtx.buffers.front();
        gTaskCtx.buffers.pop_front();
        nv_image_reader_enqueue(gTaskCtx.reader, &nv_image, nv_image.index);
        pthread_mutex_unlock(&gTaskCtx.buffer_mutex);
        index = nv_image.index;
        rx_image.width  = nv_image.width;
        rx_image.height = nv_image.height;
        rx_image.format = ROCKX_PIXEL_FORMAT_YUV420SP_NV12;
        rx_image.buffer.size     = nv_image.buffer.size;
        rx_image.buffer.virtAddr = nv_image.buffer.virt_addr;
        input->packets[0].image  = rx_image;
        // ALOGD("consume buffer[%d]: %p %dx%d", index, nv_image.buffer.virt_addr, nv_image.width, nv_image.height);
    }

    image = &(input->packets[0].image);
    // ALOGD("process buffer[%d]: %p %dx%d", index, image->buffer.virtAddr, image->width, image->height);
    if ((nullptr == image->buffer.virtAddr) && (0 == image->buffer.fd)) {
        ALOGD("invalid buffer[%d]: %p %dx%d", index, image->buffer.virtAddr, image->width, image->height);
        return ret;
    }

    // task inference
    int64_t time_now = nv_time_ms();
    RockXOutput* output = RockXOutputCreate(gTaskCtx.xhandle, nullptr, 0);
    ret = RockXProcess(gTaskCtx.xhandle, input, output);
    if (ret != ROCKX_RET_SUCCESS) {
        ALOGE("Failed to RockXProcess, ret=%d", ret);
        return ret;
    }

    RockXPacket* packet_objects = RockXFindPacket(output->packets, output->num, "detect_objects");
    if (packet_objects == nullptr) {
        ALOGE("Failed to RockXFindPacket");
    } else {
        auto* landmarks = (RockXBodyLandmarks*)packet_objects->value.p;

        cJSON *jsonRoot = cJSON_CreateObject();
        cJSON_AddNumberToObject(jsonRoot, "isTracked", landmarks->isTracked);

        cJSON *imageSizeObj = cJSON_CreateObject();
        cJSON_AddNumberToObject(imageSizeObj, "width", landmarks->imageSize.width);
        cJSON_AddNumberToObject(imageSizeObj, "height", landmarks->imageSize.height);
        cJSON_AddItemToObject(jsonRoot, "imageSize", imageSizeObj);

        cJSON *landmarksArray = cJSON_CreateArray();
        int count = 0;
        for (int idx = 0; idx < landmarks->count; idx++) {
            RockXBodyLandmark landmark = landmarks->landmarks[idx];
            float avg_score = rockx_task_avg_scores(&landmark);
            if ((landmark.score > 0.5f) && (avg_score > 0.5f)) {
                count++;
                cJSON_AddItemToArray(landmarksArray,RockXBodyLandmarkToJSON(&landmark));
            } else {
                // ALOGE("filter out landmarks[%d], score=%f avg_score=%f", idx, landmark.score, avg_score);
            }
        }
        // ALOGE("landmarksArray landmarks->count=%d count=%d", landmarks->count, count);
        cJSON_AddItemToObject(jsonRoot, "count", cJSON_CreateNumber(count));
        cJSON_AddItemToObject(jsonRoot, "landmarks", landmarksArray);

        // sizeof(json_objects) = sizeof(Landmarks)*count ≈ 1200*count
        // cJSON_PrintPreallocated(jsonRoot, json_objects, 2048, 1);     // bad code, fixed size
        *json_objects = cJSON_Print(jsonRoot);
        cJSON_Delete(jsonRoot);
    }

    RockXInputDestroy(gTaskCtx.xhandle, input);
    RockXOutputDestroy(gTaskCtx.xhandle, output);

    return ROCKX_RET_SUCCESS;
}

RockXRetCode rockx_task_destory() {
    if (nullptr != gTaskCtx.xhandle) {
        RockXDestroy(gTaskCtx.xhandle);
    }
    return ROCKX_RET_SUCCESS;
}