#ifndef ROCKX_GRAPH_BUILDER_H_
#define ROCKX_GRAPH_BUILDER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rockx_type.h"
#include "rockx_container.h"

/**
 * @brief 输入列表
 * 
 */
typedef RockXArrayList RockXInputsInfo;

/**
 * @brief 输出列表
 * 
 */
typedef RockXArrayList RockXOutputsInfo;

/**
 * @brief 共享数据列表
 * 
 */
typedef RockXArrayList RockXSharesInfo;

/**
 * @brief 配置列表
 * 
 */
typedef RockXArrayList RockXParamsInfo;

/**
 * @brief 节点列表
 * 
 */
typedef RockXArrayList RockXNodesInfo;

/**
 * @brief 节点信息，节点为模块的一个实例
 * 
 */
typedef struct RockXNodeInfo {
    const char* name;                   //< 节点名称
    const char* module;                 //< 模块名称
    RockXInputsInfo inputs;             //< 输入列表
    RockXOutputsInfo outputs;           //< 输出列表
    RockXSharesInfo shares;             //< 共享数据列表
    RockXParamsInfo params;           //< 配置列表
    RockXNodesInfo nodes;               //< 子节点列表
} RockXNodeInfo;

/**
 * @brief 计算图信息，计算图由多个节点组成
 * 
 */
typedef struct RockXGraphInfo {
    const char* name;                   //< 计算图名称
    RockXInputsInfo inputs;             //< 输入列表
    RockXOutputsInfo outputs;           //< 输出列表
    RockXParamsInfo params;           //< 配置列表
    RockXNodesInfo nodes;               //< 节点列表
} RockXGraphInfo;

/**
 * @brief 创建一个计算图
 * 
 * @param name 计算图名称
 * @return RockXGraphInfo* 计算图信息
 */
RockXGraphInfo* RockXBuilderGraphCreate(const char* name);

/**
 * @brief 销毁一个计算图
 * 
 * @param graph 计算图信息
 * @return int 
 */
int RockXBuilderGraphDestroy(RockXGraphInfo* graph);

/**
 * @brief 创建一个节点
 * 
 * @param name 节点名称
 * @param module 模块名称
 * @return RockXNodeInfo* 节点信息
 */
RockXNodeInfo* RockXBuilderNodeCreate(const char* name, const char* module);

/**
 * @brief 销毁一个节点
 * 
 * @param node 节点信息
 * @return int 
 */
int RockXBuilderNodeDestroy(RockXNodeInfo* node);

/**
 * @brief 添加一个节点
 * 
 * @param nodes 节点列表
 * @param node 节点信息
 * @return int 
 */
int RockXBuilderAddNode(RockXNodesInfo nodes, RockXNodeInfo* node);

/**
 * @brief 删除一个节点
 * 
 * @param nodes 节点列表
 * @param name 节点名称
 * @return int 
 */
int RockXBuilderRemoveNode(RockXNodesInfo nodes, const char* name);

/**
 * @brief 计算图添加一个输入
 * 
 * @param graph 计算图信息
 * @param name 输入名称
 * @return int 
 */
int RockXBuilderGraphAddInput(RockXGraphInfo* graph, const char* name);

/**
 * @brief 计算图添加一个输出
 * 
 * @param graph 计算图信息
 * @param name 输出名称
 * @return int 
 */
int RockXBuilderGraphAddOutput(RockXGraphInfo* graph, const char* name);

/**
 * @brief 节点添加一个输入
 * 
 * @param node 节点信息
 * @param internalName 内部名称（模块内定义的名称）
 * @param externalName 外部名称（用户定义的名称，计算图内使用）
 * @return int 
 */
int RockXBuilderNodeAddInput(RockXNodeInfo* node, const char* internalName, const char* externalName);

/**
 * @brief 节点输入获取
 * 
 * @param node 节点信息
 * @param internalName 内部名称（模块内定义的名称）
 * @return const char* 外部名称（用户定义的名称，计算图内使用）
 */
const char* RockXBuilderNodeGetInput(const RockXNodeInfo* node, const char* internalName);

/**
 * @brief 节点添加一个输出
 * 
 * @param node 节点信息
 * @param internalName 内部名称（模块内定义的名称）
 * @param externalName 外部名称（用户定义的名称，计算图内使用）
 * @return int 
 */
int RockXBuilderNodeAddOutput(RockXNodeInfo* node, const char* internalName, const char* externalName);

/**
 * @brief 节点输出获取
 * 
 * @param node 节点信息
 * @param internalName 内部名称（模块内定义的名称）
 * @return const char* 外部名称
 */
const char* RockXBuilderNodeGetOutput(const RockXNodeInfo* node, const char* internalName);

/**
 * @brief 节点添加一个共享数据
 * 
 * @param node 节点信息
 * @param internalName 内部名称（模块内定义的名称）
 * @param externalName 外部名称（用户定义的名称，计算图内使用）
 * @return int 
 */
int RockXBuilderNodeAddShare(RockXNodeInfo* node, const char* internalName, const char* externalName);

/**
 * @brief 节点共享数据获取
 * 
 * @param node 节点信息
 * @param internalName 内部名称（模块内定义的名称）
 * @return const char* 外部名称
 */
const char* RockXBuilderNodeGetShare(const RockXNodeInfo* node, const char* internalName);

/**
 * @brief 添加一个配置
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @param value 配置值
 * @param needRelease 是否需要释放
 * @return int 
 */
int RockXBuilderAddParam(RockXParamsInfo params, const char* key, const RockXValue* value, int needRelease);

/**
 * @brief 添加一个整型配置
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @param value 配置值
 * @return int 
 */
int RockXBuilderAddParamInt(RockXParamsInfo params, const char* key, int value);

/**
 * @brief 添加一个浮点类型配置
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @param value 配置值
 * @return int 
 */
int RockXBuilderAddParamFloat(RockXParamsInfo params, const char* key, float value);

/**
 * @brief 添加一个字符串类型配置
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @param value 配置值
 * @param iscopy 是否拷贝
 * @param needRelease 是否需要释放
 * @return int 
 */
int RockXBuilderAddParamString(RockXParamsInfo params, const char* key, const char* value, int iscopy, int needRelease);

/**
 * @brief 添加一个指针类型配置
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @param value 配置值
 * @param needRelease 是否需要释放
 * @return int 
 */
int RockXBuilderAddParamPointer(RockXParamsInfo params, const char* key, const void* value, int needRelease);

/**
 * @brief 检查配置是否存在
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @return int 1: 存在；0：不存在
 */
int RockXBuilderCheckParamExist(const RockXParamsInfo params, const char* key);

/**
 * @brief 获取一个配置
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @return RockXValue* 配置值
 */
RockXValue* RockXBuilderGetParam(const RockXParamsInfo params, const char* key);

/**
 * @brief 获取一个整型配置
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @param defval 默认值
 * @return int 
 */
int RockXBuilderGetParamInt(const RockXParamsInfo params, const char* key, int defval);

/**
 * @brief 获取一个字符串类型配置
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @param defval 默认值
 * @return const char* 
 */
const char* RockXBuilderGetParamString(const RockXParamsInfo params, const char* key, const char* defval);

/**
 * @brief 获取一个指针类型配置
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @param defval 默认值
 * @return void* 
 */
void* RockXBuilderGetParamPointer(const RockXParamsInfo params, const char* key, void* defval);

/**
 * @brief 获取一个浮点类型配置
 * 
 * @param params 配置列表
 * @param key 配置名称
 * @param defval 默认值
 * @return float 
 */
float RockXBuilderGetParamFloat(const RockXParamsInfo params, const char* key, float defval);

/**
 * @brief 将计算图信息转换为节点信息
 * 
 * @param graphInfo 计算图信息
 * @return RockXNodeInfo* 节点信息
 */
RockXNodeInfo* RockXBuilderGraphInfoMoveToNodeInfo(RockXGraphInfo* graphInfo);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  //  ROCKX_GRAPH_BUILDER_H_