#ifndef ROCKX_IMAGE_UTILS
#define ROCKX_IMAGE_UTILS

#include "rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 根据图像宽高像素格式信息获取图像Buffer内存大小
 * 
 * @param image 图像
 * @return size_t 内存大小
 */
size_t RockXImageSize(RockXImage* image);

/**
 * @brief 根据图像宽高像素格式信息分配图像Buffer内存
 * 
 * @param image 图像
 * @param type 内存类型
 * @return RockXRetCode 
 */
RockXRetCode RockXImageAllocBuffer(RockXImage* image, RockXMemType type);

/**
 * @brief 释放图像Buffer内存
 * 
 * @param image 图像
 * @return RockXRetCode 
 */
RockXRetCode RockXImageFreeBuffer(RockXImage* image);

/**
 * @brief 同步图像Buffer内存，Cache同步到内存
 * 
 * @param image 图像
 * @return RockXRetCode 
 */
RockXRetCode RockXImageSyncBuffer(RockXImage* image);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif