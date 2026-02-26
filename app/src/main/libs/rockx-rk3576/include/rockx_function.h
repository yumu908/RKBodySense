#ifndef __ROCKX_FUNCTION_H__
#define __ROCKX_FUNCTION_H__

#include "rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 函数调用
 */
typedef RockXRetCode (*RockXFunctionProcess)(RockXInput* input, RockXOutput* output);

/**
 * @brief 函数定义
 */
typedef struct {
    const char* name;
    RockXFunctionProcess call;
    int inNum;
    int outNum;
    const char** inputs;
    const char** outputs;
} RockXFunction;

/**
 * @brief 全局注册函数
 */
RockXRetCode RockXFunctionGlobalRegister(RockXFunction* function);

/**
 * @brief 全局注销函数
 */
RockXRetCode RockXFunctionGlobalUnregister(const char* name);

/**
 * @brief 查找函数
 */
RockXFunction* RockXFunctionFind(const char* name);

/**
 * @brief 调用函数
 */
RockXRetCode RockXFunctionCall(RockXFunction* function, RockXInput* input, RockXOutput* output);

/**
 * @brief 创建函数输入
 */
RockXInput* RockXFunctionInputCreate(RockXFunction* function);

/**
 * @brief 创建函数输出
 */
RockXOutput* RockXFunctionOutputCreate(RockXFunction* function);

/**
 * @brief 销毁函数输入
 */
RockXRetCode RockXFunctionInputDestroy(RockXInput* input);

/**
 * @brief 销毁函数输出
 */
RockXRetCode RockXFunctionOutputDestroy(RockXOutput* output);

#ifdef __cplusplus
}
#endif

#endif
