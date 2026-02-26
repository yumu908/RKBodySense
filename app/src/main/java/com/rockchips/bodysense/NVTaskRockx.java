package com.rockchips.bodysense;

import android.graphics.Bitmap;
import android.util.Log;

import java.nio.IntBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class NVTaskRockx {

    // Rockx native library path
    private String mModelPath;

    // Rockx model file path
    private String mModelName;

    // Rockx handle, initialized by loadModel
    private long mModelHandle = 0;

    private IntBuffer mRawFrame;

    // Thread pool for task execution
    private ExecutorService mExecutor;

    private NVTaskPerf mTaskPerf;

    private String TAG = "NVTaskRockx";
    private boolean mInitialized;
    private int mFrameNum = 0;

    OnTaskListener mTaskListener;

    public NVTaskRockx(String modelPath, String modelName) {
        this.mModelPath = modelPath;
        this.mModelName = modelName;
        // this.mExecutor = Executors.newCachedThreadPool();
        this.mExecutor = Executors.newSingleThreadExecutor();
        this.mTaskPerf = new NVTaskPerf(1000);
        mInitialized = false;
        mFrameNum = 0;
    }

    public void onCreate() {
        try {
            // System.load(mModelPath);
            if (!mInitialized) {
                NativeDetect.TaskCreate();
                mInitialized = true;
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to NativeDetect.TaskCreate()");
        }
    }

    // 监听器接口
    public interface OnTaskListener {
        void onListener(String jsonString);
    }

    public void setTaskListener(OnTaskListener onListener) {
        NativeDetect.setTaskListener(onListener);
    }

    // Process inference task
    public void processTask(ImageInfo image, InferenceCallback callback) {
        // mExecutor.submit(() -> {});
        try {
            mTaskPerf.start();
            mFrameNum++;
            mTaskPerf.start();
            String json_objects = NativeDetect.TaskProcess(null, 1280, 720);
            callback.onResult(null, json_objects);
            mTaskPerf.end();
            mTaskPerf.printStats("detect body-pose");
        } catch (Exception e) {
            // Invoke callback with error message
            callback.onError(e.getMessage());
        }
    }

    public void onDestroy() {
        NativeDetect.TaskDestory();
        mExecutor.shutdown();
    }

    // Callback interface for inference result
    public interface InferenceCallback {
        void onResult(Bitmap bitmap, String objects);
        void onError(String errorMessage);
    }

    public static class ImageInfo {
        public byte[] imgBytes;
        public int width;
        public int height;

        // 构造函数
        public ImageInfo(byte[] imgBytes, int width, int height) {
            this.imgBytes = imgBytes;
            this.width = width;
            this.height = height;
        }

        // 可以添加其他方法，例如获取图像大小、格式等
        public int getImageSize() {
            return imgBytes.length;
        }
    }
}
