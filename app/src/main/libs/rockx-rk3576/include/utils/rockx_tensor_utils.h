#ifndef ROCKX_TENSOR_UTILS
#define ROCKX_TENSOR_UTILS

#include "rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 从文件读取张量
 * 
 * @param path 文件路径
 * @param dtype 数据类型
 * @param out_tensor 输出张量
 * @return RockXRetCode 
 */
RockXRetCode RockXTensorReadFromFile(const char *path, RockXDataType dtype, RockXTensor *out_tensor);

/**
 * @brief 写张量到文件
 * 
 * @param path 文件路径
 * @param tensor 张量
 */
void RockXTensorWrite(const char *path, RockXTensor *tensor);

/**
 * @brief 写张量到txt文件
 * 
 * @param path 文件路径
 * @param tensor 张量
 */
RockXRetCode RockXTensorWriteTxt(const char *path, RockXTensor *tensor);

/**
 * @brief 获取张量内存大小
 * 
 * @param tensor 张量
 * @return size_t 内存大小
 */
size_t RockXTensorSize(RockXTensor* tensor);

/**
 * @brief 打印张量信息
 * 
 * @param tensor 张量
 */
void RockXTensorDump(RockXTensor *tensor);

/**
 * @brief 获取张量数据
 * 
 * @param tensor 张量
 * @param n 
 * @param c 
 * @param h 
 * @param w 
 * @param data 
 * @return RockXRetCode 
 */
RockXRetCode RockXTensorGetData(RockXTensor *tensor, int n, int c, int h, int w, float *data);

/**
 * @brief 获取张量数据
 * 
 * @param tensor 张量
 * @param index 索引
 * @param data 数据
 * @return RockXRetCode 
 */
RockXRetCode RockXTensorGetData1(RockXTensor *tensor, int index, float *data);

/**
 * @brief 获取张量数据
 * 
 * @param tensor 张量
 * @param n 
 * @param c 
 * @param h 
 * @param w 
 * @param data 
 * @return RockXRetCode 
 */
RockXRetCode RockXTensorGetDataInt8(RockXTensor *tensor, int n, int c, int h, int w, int8_t *data);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif