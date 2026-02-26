## 目录

```
.
├── demo                 # demo
├── readme.txt
├── rkauth_tool          # 授权工具
├── rockx_data           # 模型
└── sdk                  # 库和头文件
```

demo使用

编译

```
cd demo/command_line_demo
./build-linux-rk3576.sh
adb push install/rockx_rk3576_linux /data/
adb push ../../rockx_data /data/
```

授权

```
adb push rkauth_tool/Linux/aarch64/rkauth_tool_bin /data/
adb shell
./data/rkauth_tool_bin -u xxx -p xxx -o /data/key.lic
```

运行

```
adb shell
cd /data/rockx_rk3576_linux/demo/rockx_module_demo/
./rockx_image_demo  xxxx.jpg
```
