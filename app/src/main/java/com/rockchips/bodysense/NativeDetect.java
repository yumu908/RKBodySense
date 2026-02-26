package com.rockchips.bodysense;

public class NativeDetect {
    static {
        System.loadLibrary("native-rockx");
    }

    public static native int TaskCreate();
    public static native int TaskReset();
    public static native String TaskProcess(int[] dst, int width, int height);
    public static native int TaskDestory();
    public static native int setTaskListener(NVTaskRockx.OnTaskListener onListener);
}