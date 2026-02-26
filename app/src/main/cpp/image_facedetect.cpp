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

#include <android/log.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <deque>
#include <algorithm>
#include "image_buffer.h"
#include "image_facedetect.h"

#define TAG "RKNN-FaceDet"

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

typedef enum _NVTaskFaceType {
    TYPE_TASK_FACE_EXTRACT = 0,
    TYPE_TASK_FACE_COMPARE,
} NVTaskFaceType;

typedef struct NNRect {
    int left;
    int right;
    int top;
    int bottom;
} NNRect;

typedef struct NNAttribute {
    char *category;         ///< 属性类别
    char *label;            ///< 属性标签
    float score;            ///< 属性置信度
} NNAttribute;

typedef struct NNObject {
    int id;
    NNAttribute attributes;
    NNRect rect;
    char* feature;
    int   featureLen;
} NNObject;

typedef struct _NNDetectResult {
    NNObject *objects;
    int64_t timestamp;
    int32_t objectCount;
    int32_t frameId;
    int32_t chnId;
} NNDetectResult;

typedef struct _NVTaskContext {
    RockIvaHandle   mHandle;
    RockIvaWorkMode mWorkMode;
    NVTaskFaceType  mTaskType;
    ImageBufferManager mBufferManager;
    int mSkipNum;
    int mFaceNum;
    int mDetNum;
    std::deque<NNDetectResult*> mDetResults;
} NVTaskContext;

#define FACE_LIBRARY_NAME "face.sqlite"
#define FACE_MODEL_ROOT "/sdcard/Android/data/rockiva"

static NVTaskContext gTaskCtx;

// cond and lock for sync task
static pthread_cond_t  gTaskCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gTaskLock  = PTHREAD_MUTEX_INITIALIZER;
static int32_t         gTaskReady = 0;

static void* SyncTaskWaiter(void* args) {
    pthread_mutex_lock(&gTaskLock);
    pthread_cond_wait(&gTaskCond, &gTaskLock);
    pthread_mutex_unlock(&gTaskLock);
    return NULL;
}

static void* SyncTaskNotifier(void* args) {
    // some tasking...
    pthread_mutex_lock(&gTaskLock);
    pthread_cond_signal(&gTaskCond);
    pthread_mutex_unlock(&gTaskLock);
    return NULL;
}

static void NNDetectResultRelease(NNDetectResult *results) {
    if ((NULL == results) && (NULL != results->objects)) {
        free(results->objects);
        results->objects     = NULL;
        results->objectCount = 0;
    }
}

int ReadDataFile(const char *path, char **out_data) {
    FILE *fp = fopen(path, "rb");
    if(fp == NULL) {
        ALOGE("failed to fopen(%s)", path);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    char *data = (char *)malloc(file_size+1);
    data[file_size] = 0;
    fseek(fp, 0, SEEK_SET);
    if(file_size != fread(data, 1, file_size, fp)) {
        ALOGE("failed to fread(data=%p), uri=%s", data, path);
        free(data);
        fclose(fp);
        return -1;
    }
    if(fp) {
        fclose(fp);
    }
    *out_data = data;
    return file_size;
}

#define NN_MAX(a, b) ((a) > (b) ? (a) : (b))
int checkFaceInfo(const RockIvaImage *frame, const RockIvaFaceInfo *face_info) {
    int width  = NN_MAX(frame->info.width, 1280);
    int height = NN_MAX(frame->info.height, 720);
    int rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(width,  face_info->faceRect.topLeft.x);
    int rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(height, face_info->faceRect.topLeft.y);
    int rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(width,  face_info->faceRect.bottomRight.x);
    int rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(height, face_info->faceRect.bottomRight.y);

    if ((rect_x2 - rect_x1)*(rect_y2 - rect_y1) > 240*180) {
        ALOGD("frame(%dx%d) FaceId=%d rect(%03d-%03d-%3d-%03d) OK",
              width, height, face_info->objId, rect_x1, rect_x2, rect_y1, rect_y2);
        return 0;
    }

    ALOGD("frame(%dx%d) FaceId=%d rect(%03d-%03d-%3d-%03d) too small",
          width, height, face_info->objId, rect_x1, rect_x2, rect_y1, rect_y2);

    return -1;
}

void task_face_feature_extract(const RockIvaFaceCapResult *result) {
    const RockIvaFaceInfo* face_info = &result->faceInfo;

    NNDetectResult  *detResult = nullptr;
    if(0 == checkFaceInfo(&(result->originImage), face_info)) {
        const char* feature_data = result->faceAnalyseInfo.feature;
        const int   feature_size = result->faceAnalyseInfo.featureSize;
        detResult = (NNDetectResult*)malloc(sizeof(NNDetectResult));
        memset(detResult, 0, sizeof(NNDetectResult));
        detResult->objectCount = 1;
        detResult->objects     = (NNObject*)malloc(sizeof(NNObject)*(detResult->objectCount));
        detResult->objects[0].rect.left   = 640 - 160;
        detResult->objects[0].rect.right  = 640 + 160;
        detResult->objects[0].rect.top    = 360 - 160;
        detResult->objects[0].rect.bottom = 360 + 160;
        detResult->objects[0].id          = face_info->objId;

        detResult->objects[0].feature     = (char*)malloc(feature_size);
        detResult->objects[0].featureLen  = feature_size;
        memcpy(detResult->objects[0].feature, feature_data, feature_size);
    }

    if (nullptr != detResult) {
        if(gTaskCtx.mDetResults.empty()) {
            gTaskCtx.mDetResults.push_back(detResult);
        } else {
            float score = 0.0f;
            NNDetectResult  *last = gTaskCtx.mDetResults.back();
            ROCKIVA_FACE_FeatureCompare(last->objects[0].feature,
                                        detResult->objects[0].feature, &score);
            if (score > 0.95f) {
                ALOGE("ROCKIVA_FACE_FeatureCompare score =%f", score);
                gTaskCtx.mDetResults.push_back(detResult);
            }
        }
        gTaskCtx.mFaceNum++;
    }

    if ((nullptr != detResult) && (gTaskCtx.mFaceNum < 0)) {
        char image_uri[64] = {0};
        RockIvaImage face_image;
        RockIvaRectExpandRatio expand_ratio = {0};
        ROCKIVA_IMAGE_Crop(&result->originImage, &face_info->faceRect, 4, &face_image, &expand_ratio, ROCKIVA_IMAGE_FORMAT_RGB888);
        snprintf(image_uri, 64, "/sdcard/Pictures/image-faces/image-%d.jpg", gTaskCtx.mFaceNum);
        ROCKIVA_IMAGE_Write(image_uri, &face_image);
        ROCKIVA_IMAGE_Release(&face_image);
    }
}

void task_face_feature_compare(const RockIvaFaceCapResult *result) {
    // face feature compared
    RockIvaFaceSearchResults cmp_result;
    const char* feature_data = result->faceAnalyseInfo.feature;
    const int   feature_size = result->faceAnalyseInfo.featureSize;
    ROCKIVA_FACE_SearchFeature(FACE_LIBRARY_NAME, feature_data,
                               feature_size, 1, 5, &cmp_result);
    int cmpNum = 0;
    for (int idx = 0; idx < cmp_result.num; idx++) {
        if (0 == idx) {
            ALOGD("face compare feature. result(idx=%d face_info=%s score=%f", \
               idx, cmp_result.faceIdScore[idx].faceIdInfo, cmp_result.faceIdScore[idx].score);
        }
        if(cmp_result.faceIdScore[idx].score > 0.9f) {cmpNum++;}
    }
    if ((cmp_result.num > 0) && (cmpNum>0)) {
        ALOGE("ROCKIVA_FACE_SearchFeature top_face_num=%d", cmp_result.num);
        gTaskCtx.mFaceNum++;
    }
}

void callback_frame_release(const RockIvaReleaseFrames* releaseFrames, void* userdata) {
    for (int idx = 0; idx < releaseFrames->count; idx++) {
        const RockIvaImage* frame = &releaseFrames->frames[idx];
        IvaImageBuf* frame_buf = (IvaImageBuf*)frame->extData;
        ALOGD("callback_frame_release frame = %p", frame_buf);
        if (nullptr != frame_buf) {
            RetureImageBuffer(gTaskCtx.mBufferManager, frame_buf);
        }
    }
}

void callback_face_detect(const RockIvaFaceDetResult* result,
                          const RockIvaExecuteStatus status,
                          void* userdata) {
    for(int idx = 0; idx < result->objNum; idx++) {
        // checkFaceInfo(&(result->frame), &result->faceInfo[idx]);
    }
    SyncTaskNotifier(NULL);
}

void callback_face_analyse(const RockIvaFaceCapResults* result, \
                           const RockIvaExecuteStatus status, void* userdata) {
    if (status == ROCKIVA_SUCCESS && result->num > 0) {
        for (int idx = 0; idx < result->num; idx++) {
            const RockIvaFaceCapResult* face_result = &(result->faceResults[idx]);
            if (face_result->qualityResult != ROCKIVA_FACE_QUALITY_OK) {
                continue;
            }

            // face feature extracted
            if (gTaskCtx.mTaskType == TYPE_TASK_FACE_EXTRACT) {
                task_face_feature_extract(face_result);
            }

            // face feature compared
            if (gTaskCtx.mTaskType == TYPE_TASK_FACE_COMPARE) {
                task_face_feature_compare(face_result);
            }
        }
    }
}

static inline int64_t nv_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)(ts.tv_sec*1000 + ts.tv_nsec/1000000);
}

RockIvaRetCode rknn_task_create() {
    ALOGD("rknn_task_create");

    // Task Context
    gTaskCtx.mHandle   = nullptr;
    gTaskCtx.mWorkMode = ROCKIVA_MODE_PICTURE;
    gTaskCtx.mSkipNum  = 0;
    gTaskCtx.mFaceNum  = 0;

    // select detection-model for object detection
    RockIvaInitParam optsDetect;
    memset(&optsDetect, 0, sizeof(RockIvaInitParam));
    optsDetect.detModel = ROCKIVA_DET_MODEL_CLS8;
    // config model path and license
    char licenseUri[ROCKIVA_PATH_LENGTH];
    snprintf(optsDetect.modelPath, ROCKIVA_PATH_LENGTH, "%s", FACE_MODEL_ROOT);
    snprintf(licenseUri, ROCKIVA_PATH_LENGTH, "%s/rockiva.lic", FACE_MODEL_ROOT);

    if (access(licenseUri, R_OK) == 0) {
        char *license_data;
        int   license_size;
        license_size = ReadDataFile(licenseUri, &license_data);
        if (license_data != NULL && license_size > 0) {
            optsDetect.license.memAddr = license_data;
            optsDetect.license.memSize = license_size;
            ALOGE("license(%p size=%d)", license_data, license_size);
        }
    } else {
        ALOGE("failed to access(%s) error=%s", licenseUri, strerror(errno));
    }
    if(ROCKIVA_RET_SUCCESS != ROCKIVA_Init(&(gTaskCtx.mHandle), gTaskCtx.mWorkMode, &optsDetect, nullptr)){
        ALOGE("failed to ROCKIVA_Init");
        gTaskCtx.mHandle = nullptr;
        return ROCKIVA_RET_FAIL;
    }

    // ROCKIVA_SetFrameReleaseCallback(gTaskCtx.mHandle, callback_frame_release);

    // select face-model for object analysis
    RockIvaFaceTaskParams optsFace;
    memset(&optsFace, 0, sizeof(RockIvaFaceTaskParams));
    // face landmark
    optsFace.mode = ROCKIVA_FACE_MODE_NORMAL;
    optsFace.faceTaskType.faceAttributeEnable = 0;
    optsFace.faceTaskType.faceLandmarkEnable  = 0;
    optsFace.faceTaskType.faceLandmarkEnable  = 0;
    optsFace.faceTaskType.faceRecognizeEnable = 1;

    RockIvaFaceCallback optsCallback;
    optsCallback.detCallback     = nullptr; // callback_face_detect;
    optsCallback.analyseCallback = callback_face_analyse;
    // load face-model
    if(ROCKIVA_RET_SUCCESS != ROCKIVA_FACE_Init(gTaskCtx.mHandle, &optsFace, optsCallback)) {
        ALOGE("failed to ROCKIVA_FACE_Init");
        gTaskCtx.mHandle = nullptr;
        return ROCKIVA_RET_FAIL;
    }

    return ROCKIVA_RET_SUCCESS;
}

int rknn_task_reset(int stage) {
    if (stage < 10) {
        gTaskCtx.mSkipNum = 0;
        gTaskCtx.mFaceNum = 0;
    }

    if (stage == 100) {
        for (std::deque<NNDetectResult*>::iterator it = gTaskCtx.mDetResults.begin(); it != gTaskCtx.mDetResults.end(); ++it) {
            NNDetectResult* result = *it;
            const char* face_data = result->objects[0].feature;
            const int   face_size = result->objects[0].featureLen;
            RockIvaFaceIdInfo faceId_info;
            sprintf(faceId_info.faceIdInfo, "face-%d",result->objects[0].id);
            ROCKIVA_FACE_FeatureLibraryControl(FACE_LIBRARY_NAME, ROCKIVA_FACE_FEATURE_INSERT,
                                                        &faceId_info, 1, face_data, face_size);
            NNDetectResultRelease(result);
        }
        ALOGE("successfully record face (faceNum=%d detNum=%d)", gTaskCtx.mFaceNum, gTaskCtx.mDetNum);
        gTaskCtx.mDetResults.clear();
    }
    return gTaskCtx.mFaceNum;
}

#define ALLOC_BUFFER_TYPE 0  // 0:cpu 1:dma
RockIvaRetCode rknn_task_process_feature(RockIvaImage* image, int det_num) {
    // ALOGD("rknn_task_process_feature");
    if (nullptr == gTaskCtx.mHandle) {
        return ROCKIVA_RET_FAIL;
    }

    if ((nullptr == gTaskCtx.mBufferManager) && (nullptr != image->dataAddr)) {
        // gTaskCtx.mBufferManager = CreateImageBufferManagerPreAlloc(3, image->info.width, image->info.height,
        //                                                           ROCKIVA_IMAGE_FORMAT_RGBA8888,
        //                                                           ALLOC_BUFFER_TYPE);
    }

    gTaskCtx.mTaskType = TYPE_TASK_FACE_EXTRACT;
    gTaskCtx.mDetNum   = det_num;
    if ((gTaskCtx.mSkipNum >= 5) && (nullptr != image->dataAddr)) {
        int64_t now = nv_time_ms();
        // IvaImageBuf* frame_buf = AcquireImageBuffer(gTaskCtx.mBufferManager, 500);
        // ROCKIVA_IMAGE_Convert(image, &(frame_buf->image), ROCKIVA_IMAGE_TRANSFORM_NONE);
        // frame_buf->image.extData = frame_buf;
        ROCKIVA_PushFrame(gTaskCtx.mHandle, image, NULL);
        // SyncTaskWaiter(nullptr);
        ALOGE("ROCKIVA_PushFrame consumed = %ld ms", (nv_time_ms() - now));
        gTaskCtx.mSkipNum = 0;
    }
    gTaskCtx.mSkipNum++;

    return ROCKIVA_RET_SUCCESS;
}

RockIvaRetCode rknn_task_process_comapre(RockIvaImage* image, int det_num) {
    // ALOGD("rknn_task_process_comapre");
    if (nullptr == gTaskCtx.mHandle) {
        return ROCKIVA_RET_FAIL;
    }
    gTaskCtx.mTaskType = TYPE_TASK_FACE_COMPARE;
    gTaskCtx.mDetNum   = det_num;
    if ((gTaskCtx.mSkipNum >= 5) && (nullptr != image->dataAddr)) {
        int64_t now = nv_time_ms();
        // IvaImageBuf* frame_buf = AcquireImageBuffer(gTaskCtx.mBufferManager, 500);
        // ROCKIVA_IMAGE_Convert(image, &(frame_buf->image), ROCKIVA_IMAGE_TRANSFORM_NONE);
        // frame_buf->image.extData = frame_buf;
        ROCKIVA_PushFrame(gTaskCtx.mHandle, image, NULL);
        // SyncTaskWaiter(nullptr);
        // ALOGE("ROCKIVA_PushFrame consumed = %ld ms", (nv_time_ms() - now));
        gTaskCtx.mSkipNum = 0;
    }
    gTaskCtx.mSkipNum++;

    return ROCKIVA_RET_SUCCESS;
}

RockIvaRetCode rknn_task_destory(){
    ALOGD("rknn_task_destory");

    if (nullptr != gTaskCtx.mHandle) {
        ROCKIVA_FACE_Release(gTaskCtx.mHandle);
        ROCKIVA_Release(gTaskCtx.mHandle);
    }

    if (nullptr != gTaskCtx.mBufferManager) {
        DestroyImageBufferManager(gTaskCtx.mBufferManager);
    }

    return ROCKIVA_RET_SUCCESS;
}