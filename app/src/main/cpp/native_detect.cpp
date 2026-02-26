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
 *
 *  [YUV420Converter](https://github.com/qiuxintai/YUV420Converter)
 */

#include <android/log.h>
#include <jni.h>
#include <stdlib.h>
#include <string.h>

#include "image_detect_rockx.h"

#define TAG "DetectJni"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

using namespace std;

JavaVM*   g_task_jvm = nullptr;
jobject   g_task_listener  = nullptr;
jmethodID g_task_method_id = nullptr;

// Native 层的回调函数
void onListener(const char* jsonString, void* context) {
    JNIEnv* env = nullptr;
    if (g_task_jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        // attach this thread to JVM if this is a sub-thread
        g_task_jvm->AttachCurrentThread(&env, nullptr);
    }

    if (env && g_task_listener && g_task_method_id && jsonString) {
        jstring jJson = env->NewStringUTF(jsonString);

        env->CallVoidMethod(g_task_listener, g_task_method_id, jJson);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
        }
        env->DeleteLocalRef(jJson);
    }
    g_task_jvm->DetachCurrentThread();
}

void printClassName(JNIEnv *env, jclass clazz) {
    // 获取 Class 类
    jclass classClass = env->FindClass("java/lang/Class");
    if (!classClass) {
        ALOGD("========================= Failed to find java/lang/Class");
        return;
    }

    // 获取 getName() 方法 ID
    jmethodID getNameMethod = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
    if (!getNameMethod) {
        ALOGD("========================= Failed to get getName() method");
        return;
    }

    // 调用 getName() 获取类名字符串
    jstring nameStr = (jstring)env->CallObjectMethod(clazz, getNameMethod);
    if (!nameStr) {
        ALOGD("========================= Failed to call getName()");
        return;
    }

    // 转换为 C 字符串并打印
    const char *name = env->GetStringUTFChars(nameStr, nullptr);
    if (name) {
        ALOGD("========================= Class name: %s", name);
        ALOGD("========================= Class name: %s", name);
        ALOGD("========================= Class name: %s", name);
        env->ReleaseStringUTFChars(nameStr, name);
    }

    // 清理局部引用
    env->DeleteLocalRef(nameStr);
    env->DeleteLocalRef(classClass);
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_task_jvm = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_rockchips_bodysense_NativeDetect_TaskCreate(JNIEnv *env, jclass clazz) {
    return rockx_task_init("PoseX");
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_rockchips_bodysense_NativeDetect_TaskReset(JNIEnv *env, jclass clazz) {
    return rockx_task_reset();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_rockchips_bodysense_NativeDetect_setTaskListener(JNIEnv *env, jobject clazz, jobject listener) {
    // find class of task_listener
    if (g_task_listener) {
        env->DeleteGlobalRef(g_task_listener);
    }
    g_task_listener = env->NewGlobalRef(listener);

    // find ask_method_id
    jclass listenerClass = env->GetObjectClass(listener);
    printClassName(env, listenerClass);  // 打印类名
    jclass detectClass = env->GetObjectClass(clazz);
    printClassName(env, detectClass);  // 打印类名
    g_task_method_id = env->GetMethodID(listenerClass, "onListener", "(Ljava/lang/String;)V");

    if (g_task_method_id == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/NoSuchMethodError"), "onListener method not found");
        return -1;
    }
    rockx_task_set_task_listener(onListener);
    return 0;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_rockchips_bodysense_NativeDetect_TaskProcess(JNIEnv *env, jclass clazz, \
                            jintArray bytes, jint width, jint height) {

    char *json_objects = nullptr;
    jint ret = rockx_task_process(nullptr, &json_objects);

    if (NULL != bytes) {
        jint *_bytes = env->GetIntArrayElements(bytes, NULL);
        env->ReleaseIntArrayElements(bytes, _bytes, 0);
    }

    return env->NewStringUTF(json_objects);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_rockchips_bodysense_NativeDetect_TaskDestory(JNIEnv *env, jclass clazz) {
    return rockx_task_destory();
}