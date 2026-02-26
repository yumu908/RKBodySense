//
// Created by Optical on 2025/7/29.
//

#ifndef NV_ROCKIVA_IMAGE_CONVERTER_H
#define NV_ROCKIVA_IMAGE_CONVERTER_H

#include "image_common.h"

// image format(RockXPixelFormat) is defined in rockx_type.h
// https://github.com/airockchip/librga/

/**
 * @brief LetterBox
 *
 */
typedef struct {
    int x_pad;
    int y_pad;
    float scale;
} NVLetterBox;

/**
 * @brief Convert image for resize and pixel format change
 *
 * @param src_image [in] Source Image
 * @param dst_image [out] Target Image
 * @param src_box [in] Crop rectangle on source image
 * @param dst_box [in] Crop rectangle on target image
 * @param color [in] Pading color if dst_box can not fill target image
 * @return int
 */
int nv_image_convert(NVImage* src_image, NVImage* dst_image, NVImageRect* src_box, NVImageRect* dst_box, char color);

/**
 * @brief Convert image with letterbox
 *
 * @param src_image [in] Source Image
 * @param dst_image [out] Target Image
 * @param letterbox [out] Letterbox
 * @param color [in] Fill color on target image
 * @return int
 */
int nv_image_convert_with_letterbox(NVImage* src_image, NVImage* dst_image, NVLetterBox* letterbox, char color);

#endif //NV_ROCKIVA_IMAGE_CONVERTER_H
