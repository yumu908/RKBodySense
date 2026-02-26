//
// Created by Optical on 2025/7/29.
//

#ifndef NV_ROCKIVA_IMAGE_COMMON_H
#define NV_ROCKIVA_IMAGE_COMMON_H

#include <sys/time.h>
#include <unistd.h>
#include <time.h>

/**
 * @brief Image pixel format
 *
 */
typedef enum {
    NV_IMAGE_FORMAT_GRAY8,
    NV_IMAGE_FORMAT_RGB888,
    NV_IMAGE_FORMAT_RGBA8888,
    NV_IMAGE_FORMAT_YUV420SP_NV21,
    NV_IMAGE_FORMAT_YUV420SP_NV12,
} NVImageFormat;

/**
 * @brief Memory type
 *
 */
typedef enum {
    NV_MEMORY_TYPE_UNKNOW,
    NV_MEMORY_TYPE_CPU,
    NV_MEMORY_TYPE_DMA_BUF,
    NV_MEMORY_TYPE_DMA_HEAP
} NVMemoryType;

/**
 * @brief Image buffer
 *
 */
typedef struct {
    NVMemoryType mem_type;
    unsigned char* virt_addr;
    void* phys_addr;
    int size;
    int fd;
    int handle;
    const char* tag;
} NVImageBuffer;

/**
 * @brief Image rectangle
 *
 */
typedef struct {
    int left;
    int top;
    int right;
    int bottom;
} NVImageRect;

typedef struct {
    int width;
    int height;
    int width_stride;
    int height_stride;
    int format;
    NVImageBuffer buffer;
    int index;
    int64_t timestamp;
} NVImage;

typedef struct {
    int     task_num;
    int64_t task_time;
    int64_t stat_time;
    int     debug_num;
} NVPerfInfo;

static inline int64_t nv_time_ms() {
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)(ts.tv_sec*1000 + ts.tv_nsec/1000000);
}

#endif //NV_ROCKIVA_IMAGE_COMMON_H
