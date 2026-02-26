package com.rockchips.bodysense.utils;

public abstract class FrameReader {
    public abstract void onResume(int width, int height);
    public abstract void onPause();

    protected OnUpdateFrameListener mOnUpdateFrameListener;

    public void setOnUpdateFrameListener(OnUpdateFrameListener onPreviewFrameListener) {
        mOnUpdateFrameListener = onPreviewFrameListener;
    }

    public interface OnUpdateFrameListener {
        void onUpdateFrame(byte[] data, int width, int height);
    }
}

