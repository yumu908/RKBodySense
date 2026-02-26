# 

## rockiva授权

```bash
rk3588_box:/sdcard/models # rkauth_tool_bin
```

## selinux文件权限检查

```bash
rk3588_box:/data # setenforce 0
rk3588_box:/data # vi /etc/selinux/config; 添加SELINUX=enforcing / disabled
rk3588_box:/data # chown media_rw:ext_data_rw /sdcard/Android/data/rockiva/rockiva.lic
rk3588_box:/data # chmod 775 /sdcard/Android/data/rockiva
rk3588_box:/data # chmod 775 /sdcard/Android/data/rockiva/rockiva.lic
rk3588_box:/data # chmod 775 /sdcard/Android/data/rockiva/*.data
```