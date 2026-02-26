#ifndef _ROCKX_MODULE_H_
#define _ROCKX_MODULE_H_

#include "rockx_type.h"
#include "rockx_builder.h"
#include "pthread.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RockXModule;

typedef void* RockXModuleContext;

/**
 * @brief 模块类型
 * 
 */
typedef enum RockXModuleType {
    ROCKX_MODULE_TYPE_ATOMIC = 0,       ///< 原子模块，最小单元
    ROCKX_MODULE_TYPE_SUBGRAPH,         ///< 子图，由多个节点（模块）组成的图
} RockXModuleType;

/**
 * @brief 参数所在的作用域
 */
typedef enum RockXParamScope {
    ROCKX_PARAM_SCOPE_GLOBAL = 0,       ///< 全局参数
    ROCKX_PARAM_SCOPE_GRAPH,            ///< 节点所在图的参数
    ROCKX_PARAM_SCOPE_NODE,             ///< 节点自身的参数
} RockXParamScope;

/**
 * @brief 模块注册回调函数定义
 * 
 */
typedef int (*OnModuleRegister)(struct RockXModule* module);

/**
 * @brief 模块反注册回调函数定义
 * 
 */
typedef int (*OnModuleUnRegister)(struct RockXModule* module);

/**
 * @brief 用于模块类型为子图，获取构建子图信息
 * 
 */
typedef int (*OnModuleBuilder)(struct RockXModule* module, RockXNodeInfo* nodeInfo);

/**
 * @brief 模块预初始化回调函数定义
 * 
 */
typedef int (*OnModulePreInit)(RockXModuleContext ctx);

/**
 * @brief 模块初始化回调函数定义
 * 
 */
typedef int (*OnModuleInit)(RockXModuleContext ctx);

/**
 * @brief 模块后初始化回调函数定义
 * 
 */
typedef int (*OnModulePostInit)(RockXModuleContext ctx);

/**
 * @brief 模块反初始化回调函数定义
 * 
 */
typedef int (*OnModuleRelease)(RockXModuleContext ctx);

/**
 * @brief 模块更新选项回调函数定义
 * 
 */
typedef int (*OnModuleParamSet)(RockXModuleContext ctx, const RockXParam* param);

/**
 * @brief 模块参数获取回调函数定义
 * 
 */
typedef int (*OnModuleParamGet)(RockXModuleContext ctx, RockXParam* param);

/**
 * @brief 模块输入获取回调函数定义，用于提前设置预分配内存或者设置数据维度信息给前级节点使用
 * 
 */
typedef int (*OnModuleInputGet)(RockXModuleContext ctx, RockXPacket* input, int index);

/**
 * @brief 模块输出获取回调函数定义，用于提前设置预分配内存或设置数据维度信息
 * 
 */
typedef int (*OnModuleOutputGet)(RockXModuleContext ctx, RockXPacket* output, int index);

/**
 * @brief 模块预处理回调函数定义
 * 
 */
typedef int (*OnModulePreProcess)(RockXModuleContext ctx, RockXInput* input, RockXOutput* output);

/**
 * @brief 模块处理回调函数定义
 * 
 */
typedef int (*OnModuleProcess)(RockXModuleContext ctx, RockXInput* input, RockXOutput* output);

/**
 * @brief 模块后处理回调函数定义
 * 
 */
typedef int (*OnModulePostProcess)(RockXModuleContext ctx, RockXInput* input, RockXOutput* output);

/**
 * @brief 模块
 * 
 */
typedef struct RockXModule {
    char* name;                         ///< 模块名
    int inNum;                          ///< 模块输入个数
    int outNum;                         ///< 模块输出个数
    int shareNum;                       ///< 模块共享数据个数
    int paramNum;                       ///< 模块参数个数
    const char** inputs;                ///< 模块输入名
    const char** outputs;               ///< 模块输出名
    const char** shares;                ///< 模块共享数据名
    const char** params;                ///< 模块参数名
    RockXModuleType type;               ///< 模块类型
    OnModuleBuilder onBuilder;          ///< 子图类型构建回调函数
    OnModuleRegister onRegister;        ///< 注册回调函数
    OnModuleUnRegister onUnRegister;    ///< 反注册回调函数
    OnModulePreInit onPreInit;          ///< 预初始化回调函数
    OnModuleInit onInit;                ///< 初始化回调函数
    OnModulePostInit onPostInit;        ///< 后初始化回调函数
    OnModuleRelease onRelease;          ///< 反初始化回调函数
    OnModuleParamSet onParamSet;        ///< 参数更新回调函数
    OnModuleParamGet onParamGet;        ///< 参数获取回调函数
    OnModuleInputGet onInputsGet;       ///< 提前获取输入Tensor信息和内存的回调函数
    OnModuleOutputGet onOutputsGet;     ///< 提前获取输入Tensor信息和内存的回调函数
    OnModuleProcess onProcess;          ///< 处理回调函数
} RockXModule;

/**
 * @brief 注册模块
 * 
 * @param handle 实例句柄
 * @param module 模块
 * @return RockXRetCode 
 */
RockXRetCode RockXModuleRegister(RockXHandle handle, RockXModule* module);

/**
 * @brief 注销模块
 * 
 * @param handle 实例句柄
 * @param moduleName 模块名称
 * @return RockXRetCode 
 */
RockXRetCode RockXModuleUnRegister(RockXHandle handle, const char* moduleName);

/**
 * @brief 全局注册模块
 * 
 * @param module 模块
 * @return RockXRetCode 
 */
RockXRetCode RockXModuleGlobalRegister(RockXModule* module);

/**
 * @brief 全局注销模块
 * 
 * @param moduleName 模块名称
 * @return RockXRetCode 
 */
RockXRetCode RockXModuleGlobalUnRegister(const char* moduleName);

/**
 * @brief 设置模块上下文自定义数据
 */
int RockXModuleSetExtData(RockXModuleContext context, void* extData);

/**
 * @brief 获取模块上下文自定义数据
 */
void* RockXModuleGetExtData(RockXModuleContext context);

/**
 * @brief 获取共享数据包
 */
RockXPacket* RockXModuleGetSharedDataAt(RockXModuleContext context, int index);

/**
 * @brief 获取模块上下文对应的模块
 */
RockXModule* RockXModuleGetModule(RockXModuleContext context);

/**
 * @brief 获取配置参数
 */
RockXParam* RockXModuleGetParam(RockXModuleContext context, RockXParamScope scope, const char* nodeName, const char* name);

/**
 * @brief 获取配置参数的整型值
 */
int RockXModuleGetParamInt(RockXModuleContext context, RockXParamScope scope, const char* nodeName, const char* name, int defval);

/**
 * @brief 获取配置参数的浮点值
 */
float RockXModuleGetParamFloat(RockXModuleContext context, RockXParamScope scope, const char* nodeName, const char* name, float defval);

/**
 * @brief 获取配置参数的字符串值
 */
char* RockXModuleGetParamString(RockXModuleContext context, RockXParamScope scope, const char* nodeName, const char* name, const char* defval);

/**
 * @brief 获取配置参数的指针值
 */
void* RockXModuleGetParamPointer(RockXModuleContext context, RockXParamScope scope, const char* nodeName, const char* name, const void* defval);

/**
 * @brief 获取模块自身参数
 */
RockXParam* RockXModuleGetParamAt(RockXModuleContext context, int index);

/**
 * @brief 获取模块自身参数的整型值
 */
int RockXModuleGetParamIntAt(RockXModuleContext context, int index, int defval);

/**
 * @brief 获取模块自身参数的浮点值
 */
float RockXModuleGetParamFloatAt(RockXModuleContext context, int index, float defval);

/**
 * @brief 获取模块自身参数的字符串值
 */
char* RockXModuleGetParamStringAt(RockXModuleContext context, int index, const char* defval);

/**
 * @brief 获取模块自身参数的指针值
 */
void* RockXModuleGetParamPointerAt(RockXModuleContext context, int index, const void* defval);

/**
 * @brief 设置参数
 */
int RockXModuleSetParam(RockXModuleContext context, RockXParamScope scope, const char* nodeName, const RockXParam* param);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif