#ifndef _ROCKX_H_
#define _ROCKX_H_

#include "rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 结果回调函数
 * 
 */
typedef void (*RockXResultCallback)(RockXHandle handle, RockXOutput* output, void* userdata);

/**
 * @brief 创建实例
 * 
 * @param flag 初始化配置标识
 * @param userdata 用户自定义数据
 * @return RockXHandle 
 */
RockXHandle RockXCreate(uint32_t flag, void* userdata);

/**
 * @brief 销毁实例
 * 
 * @param handle 实例句柄
 * @return RockXRetCode 
 */
RockXRetCode RockXDestroy(RockXHandle handle);

/**
 * @brief 加载算法SDK
 * 
 * @param handle 实例句柄
 * @param path 算法SDK路径
 * @return RockXRetCode 
 */
RockXRetCode RockXLoadSDK(RockXHandle handle, const char* path);

/**
 * @brief 实例初始化
 * 
 * @param handle 实例句柄
 * @return RockXRetCode 
 */
RockXRetCode RockXInit(RockXHandle handle);

/**
 * @brief 设置节点参数
 * 
 * @param handle 实例句柄
 * @param nodeName 节点名称
 * @param param 节点选项
 * @return RockXRetCode 
 */
RockXRetCode RockXSetParam(RockXHandle handle, const char* nodeName, const RockXParam* param);

/**
 * @brief 设置节点参数为整数
 * 
 * @param handle 实例句柄
 * @param nodeName 节点名称
 * @param paramName 参数名称
 * @param value 参数值
 * @return RockXRetCode 
 */
RockXRetCode RockXSetParamInt(RockXHandle handle, const char* nodeName, const char* paramName, int value);

/**
 * @brief 设置节点参数为长整数
 * 
 * @param handle 实例句柄
 * @param nodeName 节点名称
 * @param paramName 参数名称
 * @param value 参数值
 * @return RockXRetCode 
 */
RockXRetCode RockXSetParamLong(RockXHandle handle, const char* nodeName, const char* paramName, long value);

/**
 * @brief 设置节点参数为浮点数
 * 
 * @param handle 实例句柄
 * @param nodeName 节点名称
 * @param paramName 参数名称
 * @param value 参数值
 * @return RockXRetCode 
 */
RockXRetCode RockXSetParamFloat(RockXHandle handle, const char* nodeName, const char* paramName, float value);

/**
 * @brief 设置节点参数为字符串
 * 
 * @param handle 实例句柄
 * @param nodeName 节点名称
 * @param paramName 参数名称
 * @param value 参数值
 * @param needRelease 是否需要释放
 * @return RockXRetCode 
 */
RockXRetCode RockXSetParamString(RockXHandle handle, const char* nodeName, const char* paramName, const char* value, int needRelease);

/**
 * @brief 设置节点参数为指针
 * 
 * @param handle 实例句柄
 * @param nodeName 节点名称
 * @param paramName 参数名称
 * @param value 参数值
 * @param needRelease 是否需要释放
 * @param cb 回调函数
 * @return RockXRetCode 
 */
RockXRetCode RockXSetParamPointer(RockXHandle handle, const char* nodeName, const char* paramName, void* value, int needRelease, RockXParamCallback* cb);

/**
 * @brief 获取节点参数
 * 
 * @param handle 实例句柄
 * @param nodeName 节点名称
 * @param params 节点选项
 * @return RockXRetCode 
 */
RockXRetCode RockXGetParam(RockXHandle handle, const char* nodeName, RockXParam* params);

/**
 * @brief 更新节点状态
 * 
 * @param handle 实例句柄
 * @param nodeName 节点名称
 * @param action 动作
 * @return RockXRetCode 
 */
RockXRetCode RockXSetNodeState(RockXHandle handle, const char* nodeName, RockXNodeAction action);

/**
 * @brief 配置需要的输入，RockXInputCreate将根据配置创建输入
 * 
 * @param handle 
 * @param name 输入名称
 * @param inNames 输入名称列表
 * @param inNum 输入数量
 * @return RockXRetCode 
 */
RockXRetCode RockXSetInputInfo(RockXHandle handle, const char* name, const char* inNames[], int inNum);

/**
 * @brief 配置需要的输出，有两种执行方式：
 *      同步模式时，callback可以置空，后面调用RockXOutputCreate将根据配置创建输出
 *      异步模式时，可以设置callback回调函数和回调模式
 * 
 * @param handle 实例句柄
 * @param name 输入名称
 * @param outputName 输出名称列表
 * @param callback 回调函数
 * @param mode 回调模式
 * @return RockXRetCode 
 */
RockXRetCode RockXSetOutputInfo(RockXHandle handle, const char* name, const char* outNames[], int outNum, RockXResultCallback callback, RockXCallbackMode mode);

/**
 * @brief 
 * 
 * @param handle 实例句柄
 * @param name 输入名称
 * @return RockXRetCode 
 */
RockXRetCode RockXRemoveOutputInfo(RockXHandle handle, const char* name);

/**
 * @brief 创建输入
 * 
 * @return RockXInput* 
 */
RockXInput* RockXInputCreate(RockXHandle handle);

/**
 * @brief 释放输入
 * 
 * @param input 输入
 */
RockXRetCode RockXInputDestroy(RockXHandle handle, RockXInput* input);

/**
 * @brief 创建输出
 * 
 * @param handle 实例句柄
 * @param outNames 输出名称列表
 * @param outNum 输出数量
 * @return RockXOutput* 
 */
RockXOutput* RockXOutputCreate(RockXHandle handle, const char* outNames[], int outNum);

/**
 * @brief 释放输出
 * 
 * @param handle 实例句柄
 * @param output 输出
 * @return RockXRetCode 
 */
RockXRetCode RockXOutputDestroy(RockXHandle handle, RockXOutput* output);

/**
 * @brief 处理执行
 * 
 * @param handle 
 * @param input 输入
 * @param output 输出
 * @return RockXRetCode 
 */
RockXRetCode RockXProcess(RockXHandle handle, RockXInput* input, RockXOutput* output);

/**
 * @brief 异步处理执行
 * 
 * @param handle 
 * @param input 输入
 * @param wait 等待句柄
 * @return RockXRetCode 
 */
RockXRetCode RockXProcessAsync(RockXHandle handle, RockXInput* input, void** wait);

/**
 * @brief 等待异步执行完成
 * 
 * @param handle 实例句柄
 * @param wait 等待句柄
 * @return RockXRetCode 
 */
RockXRetCode RockXWaitFinish(RockXHandle handle, void* wait);

/**
 * @brief 获取输入数据包数量
 * 
 * @param input 输入
 * @return int 
 */
int RockXInputPacketsNum(RockXInput* input);

/**
 * @brief 获取输出数据包数量
 * 
 * @param output 输出
 * @return int 
 */
int RockXOutputPacketsNum(RockXOutput* output);

/**
 * @brief 获取输入数据包
 * 
 * @param input 输入
 * @param index 索引
 * @return RockXPacket* 
 */
RockXPacket* RockXInputGetPacketAt(RockXInput* input, int index);

/**
 * @brief 获取输出数据包
 * 
 * @param output 输出
 * @param index 索引
 * @return RockXPacket* 
 */
RockXPacket* RockXOutputGetPacketAt(RockXOutput* output, int index);

/**
 * @brief 获取输入数据包
 * 
 * @param input 输入
 * @param name 包名称
 * @return RockXPacket* 
 */
RockXPacket* RockXInputGetPacketByName(RockXInput* input, const char* name);

/**
 * @brief 获取输出数据包
 * 
 * @param output 输出
 * @param name 包名称
 * @return RockXPacket* 
 */
RockXPacket* RockXOutputGetPacketByName(RockXOutput* output, const char* name);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif