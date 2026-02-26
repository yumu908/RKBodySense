#ifndef CAPI_ROCKX_DATA_UTILS
#define CAPI_ROCKX_DATA_UTILS

#include "rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化一个数据包
 * 
 * @param packet 数据包实例
 * @param name 数据包名称
 * @param mode 数据包模式
 * @param state 数据包状态
 * @param releaseCallback 数据包释放回调
 * @param copyCallback 数据包拷贝回调
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketInit(RockXPacket* packet, const char* name, RockXPacketMode mode, RockXPacketState state,
                             RockXPacketReleaseCallback releaseCallback, RockXPacketCopyCallback copyCallback);

/**
 * @brief 拷贝一个数据包
 * 
 * @param srcPacket 源数据包
 * @param dstPacket 目标数据包
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketCopyData(RockXPacket* srcPacket, RockXPacket* dstPacket);

/**
 * @brief 共享一个数据包
 * 
 * @param srcPacket 源数据包
 * @param dstPacket 目标数据包
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketShareData(RockXPacket* srcPacket, RockXPacket* dstPacket);

/**
 * @brief 将数据包所有权（对数据释放等操作）转移给另一个数据包
 * 
 * @param srcPacket 源数据包
 * @param dstPacket 目标数据包
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketMoveData(RockXPacket* srcPacket, RockXPacket* dstPacket);

/**
 * @brief 共享释放一个数据包
 * 
 * @param packet 数据包实例
 * @param userdata 用户数据
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketSharedRelease(RockXPacket* packet, void* userdata);

/**
 * @brief 数据包数据设置为一个整数数值
 * 
 * @param packet 数据包
 * @param v 设置值
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketSetInt(RockXPacket* packet, int v);

/**
 * @brief 数据包数据设置为一个长整型数值
 * 
 * @param packet 数据包
 * @param v 设置值
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketSetLong(RockXPacket* packet, long v);

/**
 * @brief 数据包数据设置为一个双浮点数值
 * 
 * @param packet 数据包
 * @param v 设置值
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketSetDouble(RockXPacket* packet, double v);

/**
 * @brief 数据包数据设置为一个浮点数值
 * 
 * @param packet 数据包
 * @param v 设置值
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketSetFloat(RockXPacket* packet, float v);

/**
 * @brief 数据包数据设置为一个字符串
 * 
 * @param packet 数据包
 * @param v 设置值
 * @param needRelease 是否需要释放
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketSetString(RockXPacket* packet, const char* v, int needRelease);

/**
 * @brief 数据包数据设置为一个指针
 * 
 * @param packet 数据包
 * @param v 设置值
 * @param needRelease 是否需要释放
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketSetPointer(RockXPacket* packet, void* v, int needRelease, const char* typeName);

/**
 * @brief 数据包数据设置为一个张量
 * 
 * @param packet 数据包
 * @param tensor 张量
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketSetTensor(RockXPacket* packet, RockXTensor* tensor);

/**
 * @brief 数据包数据设置为一个图像
 * 
 * @param packet 数据包
 * @param image 图像
 * @return RockXRetCode 
 */
RockXRetCode RockXPacketSetImage(RockXPacket* packet, RockXImage* image);

/**
 * @brief 查找一个数据包
 * 
 * @param packets 数据包列表
 * @param num 数据包数量
 * @param name 数据包名称
 * @return RockXPacket* 
 */
RockXPacket* RockXFindPacket(RockXPacket* packets, int num, const char* name);

/**
 * @brief 创建一个配置
 * 
 * @param name 配置名称
 * @return RockXParam* 
 */
RockXParam* RockXParamCreate(const char* name);

/**
 * @brief 销毁一个配置
 * 
 * @param param 配置
 */
void RockXParamDestroy(RockXParam* param);

/**
 * @brief 设置一个值
 * 
 * @param value 值
 * @param v 设置值
 * @return RockXRetCode 
 */
RockXRetCode RockXValueSetInt(RockXValue* value, int v);

/**
 * @brief 设置一个长整型值
 * 
 * @param value 值
 * @param v 设置值
 * @return RockXRetCode 
 */
RockXRetCode RockXValueSetLong(RockXValue* value, long v);

/**
 * @brief 设置一个双浮点值
 * 
 * @param value 值
 * @param v 设置值
 * @return RockXRetCode 
 */
RockXRetCode RockXValueSetDouble(RockXValue* value, double v);

/**
 * @brief 设置一个浮点值
 * 
 * @param value 值
 * @param v 设置值
 * @return RockXRetCode 
 */
RockXRetCode RockXValueSetFloat(RockXValue* value, float v);

/**
 * @brief 设置一个字符串值
 * 
 * @param value 值
 * @param v 设置值
 * @param needRelease 是否需要释放
 * @return RockXRetCode 
 */
RockXRetCode RockXValueSetString(RockXValue* value, const char* v, int needRelease);

/**
 * @brief 设置一个指针值
 * 
 * @param value 值
 * @param v 设置值
 * @param needRelease 是否需要释放
 * @return RockXRetCode 
 */
RockXRetCode RockXValueSetPointer(RockXValue* value, void* v, int needRelease);

/**
 * @brief 获取一个整型值
 * 
 * @param value 值
 * @param defValue 默认值
 * @return int 
 */
int RockXValueGetInt(const RockXValue* value, int defValue);

/**
 * @brief 获取一个长整型值
 * 
 * @param value 值
 * @param defValue 默认值
 * @return long 
 */
long RockXValueGetLong(const RockXValue* value, long defValue);

/**
 * @brief 获取一个浮点值
 * 
 * @param value 值
 * @param defValue 默认值
 * @return float 
 */
float RockXValueGetFloat(const RockXValue* value, float defValue);

/**
 * @brief 获取一个双浮点值
 * 
 * @param value 值
 * @param defValue 默认值
 * @return double 
 */
double RockXValueGetDouble(const RockXValue* value, double defValue);

/**
 * @brief 获取一个字符串值
 * 
 * @param value 值
 * @param defValue 默认值
 * @return char* 
 */
char* RockXValueGetString(const RockXValue* value, const char* defValue);

/**
 * @brief 获取一个指针值
 * 
 * @param value 值
 * @param defValue 默认值
 * @return void* 
 */
void* RockXValueGetPointer(const RockXValue* value, const void* defValue);

/**
 * @brief 释放一个值
 * 
 * @param value 值
 * @return RockXRetCode 
 */
RockXRetCode RockXValueRelease(RockXValue* value);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif