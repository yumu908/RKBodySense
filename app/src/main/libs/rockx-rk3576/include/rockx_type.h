#ifndef _ROCKX_TYPE_H_
#define _ROCKX_TYPE_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ROCKX_KEY_CONFIG_PATH "config_path"    //< 配置计算图配置文件路径
#define ROCKX_KEY_CONFIG_STR "config_str"     //< 配置计算图配置字符串
#define ROCKX_KEY_GRAPH_NAME "graph_name"     //< 配置计算图名称
#define ROCKX_KEY_MODULE_NAME "module_name"    //< 配置节点名称
#define ROCKX_KEY_GRAPH_INFO "graph_info"     //< 配置计算图信息

#define ROCKX_KEY_DATA_PATH "data_path"        //< 配置数据路径
#define ROCKX_KEY_AUTH_CONFIG "auth_config"    //< 授权配置
#define ROCKX_KEY_LICENSE_PATH "license"    //< 授权配置

#define ROCKX_KEY_STATE "state"
#define ROCKX_KEY_INSTANCE_MODE "instance_mode"
#define ROCKX_KEY_INSTANCE_NAME "instance_name"

#define ROCKX_FLAG_ASYNC_EXECUTE_MASK          0x00000001

/**
 * @brief RockX实例
 * 
 */
typedef void* RockXHandle;

/**
 * @brief 数据类型
 * 
 */
typedef enum {
    ROCKX_DATA_TYPE_UINT8,
    ROCKX_DATA_TYPE_INT8,
    ROCKX_DATA_TYPE_UINT16,
    ROCKX_DATA_TYPE_INT16,
    ROCKX_DATA_TYPE_UINT32,
    ROCKX_DATA_TYPE_INT32,
    ROCKX_DATA_TYPE_UINT64,
    ROCKX_DATA_TYPE_INT64,
    ROCKX_DATA_TYPE_FLOAT,
    ROCKX_DATA_TYPE_DOUBLE,
    ROCKX_DATA_TYPE_FLOAT16,
    ROCKX_DATA_TYPE_RAW,
    ROCKX_DATA_TYPE_STRUCT,
    ROCKX_DATA_TYPE_MAX,
} RockXDataType;

/**
 * @brief 图像像素格式类型
 * 
 */
typedef enum {
    ROCKX_PIXEL_FORMAT_UNKNOW = 0,     ///< Unknow binary data
    ROCKX_PIXEL_FORMAT_GRAY8 ,         ///< Gray8
    ROCKX_PIXEL_FORMAT_GRAY16,         ///< Gray16
    ROCKX_PIXEL_FORMAT_RGB888,         ///< RGB888
    ROCKX_PIXEL_FORMAT_BGR888,         ///< BGR888
    ROCKX_PIXEL_FORMAT_RGBA8888,       ///< RGBA8888
    ROCKX_PIXEL_FORMAT_BGRA8888,       ///< BGRA8888
    ROCKX_PIXEL_FORMAT_YUV420P_YU12,   ///< YUV420P YU12: YYYYYYYYUUVV
    ROCKX_PIXEL_FORMAT_YUV420P_YV12,   ///< YUV420P YV12: YYYYYYYYVVUU
    ROCKX_PIXEL_FORMAT_YUV420SP_NV12,  ///< YUV420SP NV12: YYYYYYYYUVUV
    ROCKX_PIXEL_FORMAT_YUV420SP_NV21,  ///< YUV420SP NV21: YYYYYYYYVUVU
    ROCKX_PIXEL_FORMAT_YUV422P_YU16,   ///< YUV422P YU16: YYYYYYYYUUUUVVVV
    ROCKX_PIXEL_FORMAT_YUV422P_YV16,   ///< YUV422P YV16: YYYYYYYYVVVVUUUU
    ROCKX_PIXEL_FORMAT_YUV422SP_NV16,  ///< YUV422SP NV16: YYYYYYYYUVUVUVUV
    ROCKX_PIXEL_FORMAT_YUV422SP_NV61,  ///< YUV422SP NV61: YYYYYYYYVUVUVUVU
    ROCKX_PIXEL_FORMAT_YUV422_YUYV,    ///< YUV422 YUYV: YUYVYUYV
    ROCKX_PIXEL_FORMAT_YUV422_YVYU,    ///< YUV422 YVYU: YVYUYVYU
    ROCKX_PIXEL_FORMAT_MAX,
} RockXPixelFormat;

/**
 * @brief 图像转换类型
 * 
 */
typedef enum {
    ROCKX_IMAGE_TRANSFORM_NONE              = 0x00,  /* 正常 */
    ROCKX_IMAGE_TRANSFORM_FLIP_H            = 0x01,  /* 水平翻转 */
    ROCKX_IMAGE_TRANSFORM_FLIP_V            = 0x02,  /* 垂直翻转 */
    ROCKX_IMAGE_TRANSFORM_ROTATE_90         = 0x04,  /* 顺时针90度 */
    ROCKX_IMAGE_TRANSFORM_ROTATE_180        = 0x03,  /* 顺时针180度 */
    ROCKX_IMAGE_TRANSFORM_ROTATE_270        = 0x07,  /* 顺时针270度 */
    ROCKX_IMAGE_TRANSFORM_TRANSPOSE         = 0x05,  /* 转置 */
} RockXImageTransform;

/**
 * @brief Tensor Format
 */
typedef enum {
    ROCKX_TENSOR_FORMAT_NCHW = 0,
    ROCKX_TENSOR_FORMAT_NHWC,
    ROCKX_TENSOR_FORMAT_NC1HWC2,
    ROCKX_TENSOR_FORMAT_MAX
} RockXTensorFormat;

/**
 * @brief 返回值定义
 * 
 */
typedef enum {
    ROCKX_RET_SUCCESS = 0,
    ROCKX_RET_FAIL = -1,
    ROCKX_RET_PARAM_ERR = -2,
    ROCKX_RET_INIT_ERROR = -3,
    ROCKX_RET_MEM_ERR = -4,
} RockXRetCode;

/**
 * @brief 数据包状态
 * 
 */
typedef enum RockXPacketState {
    ROCKX_PACKET_STATE_NONE = 0,    // 初始状态
    ROCKX_PACKET_STATE_DOING,       // 数据包正在处理中
    ROCKX_PACKET_STATE_READY,       // 数据包处理完成
    ROCKX_PACKET_STATE_OUTPUT,      // 数据包已输出
    ROCKX_PACKET_STATE_RELEASE,     // 数据包已释放
    ROCKX_PACKET_STATE_ERROR,       // 数据包处理出错
    ROCKX_PACKET_STATE_INVALID,     // 数据包无效
} RockXPacketState;

/**
 * @brief 更新节点状态行为
 * 
 */
typedef enum RockXNodeAction {
    ROCKX_NODE_ACTION_START,    // 启动节点
    ROCKX_NODE_ACTION_STOP,     // 停止节点
    ROCKX_NODE_ACTION_PAUSE,    // 暂停节点
    ROCKX_NODE_ACTION_RESUME,   // 恢复节点
    ROCKX_NODE_ACTION_RESET,    // 重置节点
    ROCKX_NODE_ACTION_PASS,     // 跳过节点
} RockXNodeAction;

/**
 * @brief 模块实例模式
 * 
 */
typedef enum RockXModuleInstanceMode {
    ROCKX_MODULE_INSTANCE_MODE_DEFAULT = 0,        // 默认模式，每次都会创建新的模块实例
    ROCKX_MODULE_INSTANCE_MODE_REUSE = 1,         // 复用模式，每次都会复用已有的模块实例
} RockXModuleInstanceMode;

/**
 * @brief 回调结果模式
 * 
 */
typedef enum RockXCallbackMode {
    ROCKX_CALLBACK_NONE = 0,                     // 不需要回调
    ROCKX_CALLBACK_ON_SCHED_THREAD = 1,          // 异步结果回调在调度线程中，会阻塞住当前帧的运行
    ROCKX_CALLBACK_ON_SEPARATE_THREAD = 2,       // 异步结果回调在一个单独线程中
} RockXCallbackMode;

/**
 * @brief Tensor quantization type
 * 
 */
typedef enum {
    ROCKX_TENSOR_QNT_NONE = 0,                           /* none. */
    ROCKX_TENSOR_QNT_DFP,                                /* dynamic fixed point. */
    ROCKX_TENSOR_QNT_AFFINE_ASYMMETRIC,                  /* asymmetric affine. */
    ROCKX_TENSOR_QNT_MAX
} RockXTensorQntType;

/**
 * @brief 内存类型
 * 
 */
typedef enum {
    ROCKX_MEM_TYPE_UNKNOW,
    ROCKX_MEM_TYPE_CPU,
    ROCKX_MEM_TYPE_DMA_BUF,
    ROCKX_MEM_TYPE_DMA_HEAP
} RockXMemType;

/**
 * @brief 缓冲区内存
 * 
 */
typedef struct RockXMemoryBuffer {
    RockXMemType type;
    size_t size;
    void* virtAddr;
    void* phyAddr;
    int fd;
    int handle;
    const char* tag;
} RockXMemoryBuffer;

/**
 * @brief Tensor quantization info
 * 
 */
typedef struct {
    RockXTensorQntType qntType;     ///< the quantitative type of tensor. */
    int8_t fl;                      ///< fractional length for RKNN_TENSOR_QNT_DFP. */
    int32_t zp;                     ///< zero point for RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC. */
    float scale;                    ///< scale for RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC. */
} RockXTensorQntInfo;

/**
 * @brief Tensor
 * 
 */
typedef struct RockXTensor {
    int dims[16];                          ///< 维度
    int ndims;                             ///< 维度数量
    size_t nelems;                         ///< 元素数量
    RockXDataType dtype;                   ///< 数据类型
    RockXTensorFormat format;              ///< 数据排列格式
    RockXMemoryBuffer buffer;              ///< 缓冲区
    RockXTensorQntInfo qntInfo;            ///< 量化信息
} RockXTensor;

/**
 * @brief 图像
 * 
 */
typedef struct RockXImage {
    int width;
    int height;
    RockXPixelFormat format;
    RockXMemoryBuffer buffer;
} RockXImage;

/**
 * @brief 键值对
 * 
 */
typedef struct RockXKeyValue {
    char* key;                          ///< 键
    char* value;                        ///< 值
} RockXKeyValue;

/**
 * @brief 数值类型
 * 
 */
typedef enum RockXValueType {
    ROCKX_VALUE_TYPE_NONE,             ///< 无类型
    ROCKX_VALUE_TYPE_NUMBER,           ///< 数值
    ROCKX_VALUE_TYPE_STRING,           ///< 字符串
    ROCKX_VALUE_TYPE_ARRAY,            ///< 数组
    ROCKX_VALUE_TYPE_POINTER,          ///< 指针
} RockXValueType;

/**
 * @brief 数值
 * 
 */
typedef struct RockXValue {
    RockXValueType type;    ///< 值类型
    RockXDataType dtype;    ///< 数据类型
    union {
        void* p;            ///< 指针
        char* s;            ///< 字符串
        int i;              ///< 整数
        long l;             ///< 长整数
        float f;            ///< 浮点数
        double d;           ///< 双精度浮点数
    };
    size_t count;           ///< 数量
    size_t size;            ///< 占用内存大小
    uint8_t needRelease;    ///< 是否需要调用free释放p指向的内存或者s指向的字符串
} RockXValue;

/**
 * @brief 数值数组
 * 
 */
typedef struct RockXValueArray {
    int num;
    RockXValue* values;
} RockXValueArray;

/**
 * @brief 数据包类型
 * 
 */
typedef enum RockXPacketType {
    ROCKX_PACKET_TYPE_NONE,      ///< 无类型
    ROCKX_PACKET_TYPE_IMAGE,     ///< 图像
    ROCKX_PACKET_TYPE_TENSOR,    ///< 张量
    ROCKX_PACKET_TYPE_VALUE,     ///< 数值
} RockXPacketType;

struct RockXPacket;

/**
 * @brief 数据包释放函数
 * 
 */
typedef void (*RockXPacketReleaseCallback)(struct RockXPacket* packet, void* userdata);

/**
 * @brief 数据包拷贝函数
 * 
 */
typedef void (*RockXPacketCopyCallback)(struct RockXPacket* src, struct RockXPacket* dst);

/**
 * @brief 数据包dump函数
 * 
 */
typedef void (*RockXPacketDumpCallback)(const char* path, struct RockXPacket* packet, void* userdata);

/**
 * @brief 数据包数据管理模式
 * 
 */
typedef enum {
    ROCKX_PACKET_MODE_COPY,     // 默认
    ROCKX_PACKET_MODE_SHARED,   // 支持多个Packet共享数据，使用RockXPacketShareData共享数据管理
    ROCKX_PACKET_MODE_UNIQUE,   // 只能有一个Packet独占数据，使用RockXPacketMoveData转移数据管理
} RockXPacketMode;

/**
 * @brief 数据包回调
 * 
 */
typedef struct RockXPacketCallback {
    RockXPacketReleaseCallback OnRelease;
    RockXPacketCopyCallback OnCopy;
    RockXPacketDumpCallback OnDump;
} RockXPacketCallback;

/**
 * @brief 数据包
 * 
 */
typedef struct RockXPacket {
    char* name;                         ///< 名称
    int channel;                        ///< 通道
    unsigned long index;                ///< 索引
    unsigned long timestamp;            ///< 时间戳
    char* typeName;                     ///< 类型名称
    RockXPacketType type;               ///< 类型
    union {
        RockXImage image;                ///< 图像
        RockXTensor tensor;              ///< 张量
        RockXValue value;                ///< 数值
    };
    void* extData;                        ///< 扩展数据
    RockXPacketState state;               ///< 状态
    RockXPacketMode mode;                 ///< 模式
    RockXPacketCallback cb;               ///< 回调
    void* shared;                           ///< 共享数据
} RockXPacket;

struct RockXParam;

/**
 * @brief 参数释放函数
 * 
 */
typedef void (*RockXParamReleaseCallback)(struct RockXParam* param);

/**
 * @brief 数据包拷贝函数
 * 
 */
typedef void (*RockXParamCopyCallback)(struct RockXParam* src, struct RockXParam* dst);

/**
 * @brief 数据包dump函数
 * 
 */
typedef void (*RockXParamDumpCallback)(const char* path, struct RockXParam* param, void* userdata);

typedef struct RockXParamCallback {
    RockXParamReleaseCallback OnRelease;
    RockXParamCopyCallback OnCopy;
    RockXParamDumpCallback OnDump;
} RockXParamCallback;

/**
 * @brief 参数
 * 
 */
typedef struct RockXParam {
    char* name;                          ///< 名称
    RockXValue value;                    ///< 值
    RockXParamCallback cb;               ///< 参数回调操作
    void* userdata;                      ///< 用户数据
} RockXParam;

/**
 * @brief 输入数据
 * 
 */
typedef struct RockXInput {
    const char* name;                    ///< 名称
    int num;                             ///< 数量
    RockXPacket* packets;                ///< 数据包
    void* extdata;                       ///< 扩展数据
} RockXInput;

/**
 * @brief 输出数据
 * 
 */
typedef struct RockXOutput {
    const char* name;                    ///< 名称
    int errorCode;                       ///< 错误码
    int num;                             ///< 数量
    RockXPacket* packets;                ///< 数据包
    void* userdata;                      ///< 用户数据
    void* privdata;                      ///< 私有数据
    int needRelease;                     ///< 是否需要释放
} RockXOutput;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif