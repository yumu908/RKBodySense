/*
 * Copyright 2023 Martin.Cheng
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NV_ROCKIVA_IMAGE_FACEDETECT_H
#define NV_ROCKIVA_IMAGE_FACEDETECT_H

#include "rockiva_common.h"
#include "rockiva_image.h"
#include "rockiva_det_api.h"
#include "rockiva_face_api.h"

RockIvaRetCode rknn_task_create();
int rknn_task_reset(int stage);
RockIvaRetCode rknn_task_process_feature(RockIvaImage* image, int det_num);
RockIvaRetCode rknn_task_process_comapre(RockIvaImage* image, int det_num);
RockIvaRetCode rknn_task_destory();

#endif // NV_ROCKIVA_IMAGE_FACEDETECT_H
