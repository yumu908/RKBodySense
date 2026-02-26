//
// Created by Optical on 2025/7/30.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>

#include "image_reader.h"
#include "image_converter.h"

#define TAG "main_test"

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

int parse_arguments(int argc, char *argv[], int *width, int *height, char *dev_name);

// v4l2_test -w 1280 -h 720 -d /dev/video12
int main(int argc, char *argv[]) {
    int width = 1280;
    int height = 720;
    char dev_name[24] = "/dev/video12";

    // parser parameters
    if (parse_arguments(argc, argv, &width, &height, dev_name) != 0) {
        // 如果解析失败，parse_arguments 函数内部应该已经打印了错误信息
        ALOGE("Usage: %s -w <width> -h <height> -d <dev_name>", argv[0]);
        return EXIT_FAILURE;
    }

    // 解析成功，打印解析出的值
    ALOGE("Parsed parameters:");
    ALOGE("width=%d height=%d dev_name=%s", width, height, dev_name);

    NVImageReader* reader = nv_image_reader_reader(NV_READER_TYPE_V4L2);
    nv_image_reader_open(reader, dev_name, width, height);

    int buf_index = 0;
    int64_t    task_time = nv_time_ms();
    NVPerfInfo task_perf = {0};
    for(int idx = 0; idx < 60*100; idx++) {
        NVImage image = {0};
        int64_t work_time = nv_time_ms();
        buf_index = nv_image_reader_dequeue(reader, &image);
        ALOGD("dequeue buffer[%d]: %p %dx%d", buf_index, image.buffer, image.width, image.height);
        if (buf_index >= 0) {
            nv_image_reader_enqueue(reader, &image, buf_index);
            ALOGD("enqueue buffer[%d]: %p %dx%d", buf_index, image.buffer, image.width, image.height);
        }
        task_perf.task_time += (nv_time_ms() - work_time);
        task_perf.task_num++;
        if ((nv_time_ms() - task_time) > 1000) {
            task_time = nv_time_ms();
            printf("camera is %dFPS, consume_time=%lldms/s\n", \
                           task_perf.task_num, task_perf.task_time/task_perf.task_num);
            task_perf.task_time = 0;
            task_perf.task_num  = 0;
        }
    }
    nv_image_reader_close(reader);
}

int parse_arguments(int argc, char *argv[], int *width, int *height, char *dev_name) {
    int i = 1;  // 从 argv[1] 开始，跳过程序名 argv[0]

    // Iterate through all arguments
    while (i < argc) {
        if (strcmp(argv[i], "-w") == 0) {
            if (i + 1 >= argc) {
                ALOGE("Error: -w requires a width value.");
                return -1;
            }
            *width = atoi(argv[i + 1]);
            if (*width <= 0) {
                ALOGE("Error: Width must be a positive integer.");
                return -1;
            }
            i += 2;
        } else if (strcmp(argv[i], "-h") == 0) {
            if (i + 1 >= argc) {
                ALOGE("Error: -h requires a height value.");
                return -1;
            }
            *height = atoi(argv[i + 1]);
            if (*height <= 0) {
                ALOGE("Error: Height must be a positive integer.");
                return -1;
            }
            i += 2;
        } else if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 >= argc) {
                ALOGE("Error: -s requires a scale value.");
                return -1;
            }
            sprintf(dev_name, "%s", argv[i + 1]);
            i += 2;
        } else {
            // unknown parameters
            ALOGE("Error: Unknown argument '%s'", argv[i]);
            return -1;
        }
    }

    if (*width == 0 || *height == 0) {
        ALOGE("Error: Both width (-w) and height (-h) are required.");
        return -1;
    }

    return 0;
}
