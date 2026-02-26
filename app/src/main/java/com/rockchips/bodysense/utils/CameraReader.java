package com.rockchips.bodysense.utils;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.util.Log;
import android.util.Range;
import android.util.Size;
import android.view.Surface;

import androidx.annotation.NonNull;

import java.util.Arrays;

public class CameraReader extends FrameReader {
    private static final String TAG = "CameraReader";
    private Context mContext;
    private final CameraManager mCameraManager;
    private CameraDevice mCameraDevice;
    private CameraCaptureSession mCaptureSession;
    private ImageReader mImageReader;
    private Surface mSurface;
    private String mCameraId;
    private int mViewWidth;
    private int mViewHeight;

    private Range<Integer> mFpsRange;

    public CameraReader(Context context, Surface previewSurface) {
        mContext = context;
        mCameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        mSurface = previewSurface;
    }

    @Override
    public void onResume(int width, int height) {
        mViewWidth  = width;
        mViewHeight = height;
        Log.d(TAG, "onResume(). try to resume CaptureSession");
        try {
            // try to resume stream, camera device is opened.
            if ((null != mCaptureSession) && (null != mCameraDevice)) {
                Log.d(TAG, "onResume(). try to resume stream, camera device is opened.");
                CaptureRequest.Builder builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                builder.addTarget(mImageReader.getSurface());
                if (null != mFpsRange) {
                    builder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, mFpsRange);
                }
                builder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                builder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);
                mCaptureSession.setRepeatingRequest(builder.build(), null, null);
            }

            // try to open camera device
            if (null == mCameraDevice) {
                Log.d(TAG, "onResume(). try to open camera device");
                openCamera();
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onPause() {
        Log.d(TAG, "CameraReader.onPause(). try to pause CaptureSession");
        // closeCamera();
        if (mCaptureSession != null) {
            try {
                mCaptureSession.stopRepeating();
            } catch (CameraAccessException e) {
                throw new RuntimeException(e);
            }
        }
    }

    @SuppressLint("MissingPermission")
    private void openCamera() {
        try {
            mCameraId = getUsbCameraId();
            CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(mCameraId);
            mCameraManager.openCamera(mCameraId, mCameraDeviceCallback, null);

            Range<Integer>[] fpsRanges = characteristics.get(CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
            if (fpsRanges != null) {
                mFpsRange = null;
                for (Range<Integer> range : fpsRanges) {
                    if (range.getLower() <= 30 && range.getUpper() >= 30) {
                        mFpsRange = range;
                    }
                    Log.e(TAG, "fps-range = " + range.toString());
                }
            }
        } catch (CameraAccessException e) {
            Log.e(TAG, "Opening camera (ID: " + mCameraId + ") failed.");
            e.printStackTrace();
        }
    }

    private void closeCamera() {
        if (mCaptureSession != null) {
            mCaptureSession.close();
            mCaptureSession = null;
        }
        if (mCameraDevice != null) {
            mCameraDevice.close();
            mCameraDevice = null;
        }
        if (mImageReader != null) {
            mImageReader.close();
            mImageReader = null;
        }
    }

    private String getUsbCameraId() throws CameraAccessException {
        CameraManager manager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        try {
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraId);
                StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                int orientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
                mCameraId = cameraId;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return mCameraId;
    }

    private void startCaptureSession() {
        Size camSize = new Size(1280, 720);
        Log.d(TAG, "CameraSize: " + camSize.toString());
        mImageReader = ImageReader.newInstance(camSize.getWidth(), camSize.getHeight(), ImageFormat.YUV_420_888, 3);
        mImageReader.setOnImageAvailableListener(new ImageReader.OnImageAvailableListener() {
            @Override
            public void onImageAvailable(ImageReader reader) {
                byte[] imgbytes = null;
                Image image = reader.acquireNextImage();
                if ((null != image) && (null !=mOnUpdateFrameListener)) {
                    imgbytes = FrameFormat.getDataFromImage(image);
                    mOnUpdateFrameListener.onUpdateFrame(imgbytes, image.getWidth(), image.getHeight());
                    image.close();
                }
            }
        }, null);


        try {
            mCameraDevice.createCaptureSession(Arrays.asList(mImageReader.getSurface()), mCaptureStateCallback, null);
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "Failed to start camera session");
        }
    }

    private final CameraDevice.StateCallback mCameraDeviceCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            mCameraDevice = camera;
            startCaptureSession();
        }
        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            mCameraDevice.close();
            mCameraDevice = null;
        }
        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            mCameraDevice.close();
            mCameraDevice = null;
        }
    };

    private final CameraCaptureSession.StateCallback mCaptureStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            if (mCameraDevice == null) {
                return;
            }
            mCaptureSession = session;
            try {
                CaptureRequest.Builder builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                builder.addTarget(mImageReader.getSurface());
                if (null != mFpsRange) {
                    builder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, mFpsRange);
                }
                builder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                builder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);
                session.setRepeatingRequest(builder.build(), null, null);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }
        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {
            Log.e(TAG, "Failed to configure capture session.");
        }
    };

    private void configTransform(Context context) {
        int rotation = 0;
        Matrix matrix = new Matrix();
        RectF viewRect = new RectF(0, 0, mViewWidth, mViewHeight);
        float centerX = viewRect.centerX();
        float centerY = viewRect.centerY();
        matrix.postRotate(0, centerX, centerY);
        // mTextureView.setTransform(matrix);
    }
}
