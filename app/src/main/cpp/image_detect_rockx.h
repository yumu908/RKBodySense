//
// Created by Optical on 2025/3/21.
//

#ifndef NV_ROCKIVA_IMAGE_DETECT_ROCKX_H
#define NV_ROCKIVA_IMAGE_DETECT_ROCKX_H

#include "rockx.h"
#include "rockx_module_type.h"

typedef void (*TaskListener)(const char* jsonString, void* context);
RockXRetCode rockx_task_init(const char* module_name);
RockXRetCode rockx_task_set_task_listener(TaskListener listener);
RockXRetCode rockx_task_reset();
RockXRetCode rockx_task_process(RockXImage* image, char** json_objects);
RockXRetCode rockx_task_destory();

#endif //NV_ROCKIVA_IMAGE_DETECT_ROCKX_H
