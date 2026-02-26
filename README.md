# RKBodySense智能体感应用

## 1 RKBodySense应用概述

- **ROCKX_MODULE_POSE_BODY身体骨骼关键点模型**: 对应pose_body_v4.data模型。
- **ROCKX_MODULE_POSE_FINGER手指关键点模型**: 对应pose_finger.data模型。
- **ROCKX_MODULE_HAND_DETECTION手部检测模型**: 对应hand_detection.data模型。
- **ROCKX_MODULE_HAND_LANDMARK手掌关键点模型**: 对应hand_landmark.data模型。

RKBodySense应用目前仅仅集成了POSE_BODY身体骨骼关键点模型，剩余3个模型目前还不支持，后续支持和完善。

**特别说明：**

SDK没有集成RKBodySense应用的构建，请使用**Android Studio**导入改源码，自行构建成RKBodySense应用。

## 2 ROCKX_SDK_VERSION_DATE软件包

参考 RK芯片对应的ROCKX_SDK_VERSION_DATE软件包。软件包包含：动态库、头文件、模型文件、授权工具和说明文档。

ROCKX_SDK_VERSION_DATE软件包的文件结构

```bash
ROCKX_SDK_VERSION_DATE$ tree
├─demo
│  └─command_line_demo             # 命令行示例
├─rkauth_tool                      # 授权工具和文档
├─rockx_data                       # 模型文件
│  └─pose_body_v4.data
│  └─pose_finger.data
│  └─hand_detection.data
│  └─hand_landmark.data
└─sdk
    └─rockx-rk3576-Android         # 头文件和动态库
      └─arm64-v8a
        └─librknnrt.so
        └─librockx.so
        └─librockx_modules.so
```

**授权说明：**

```bash
/rkauth_tool_bin --user=usr_name --passwd=usr_pwd --output="/data/rockx/rockx.lic" animal_det 
```

## 3 全链路低时延AI体感推理技术方案

《全链路低时延AI体感推理开发指南》见${SDK_WORKSPACE_HOME}/RKDocs/RKDocs/android/app/Rockchip_Developer_Guide_LowDelay_BodyPose_CN.pdf