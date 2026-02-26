package com.rockchips.bodysense.utils;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.util.Range;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import java.util.Arrays;
import java.util.Collections;

/**
 * A helper class to manage Camera2 API operations for video preview and frame acquisition.
 */
public class Camera2Reader {

    private static final String TAG = "Camera2Reader";

    // Provided member variables
    private Context mContext;
    private final CameraManager mCameraManager;
    private CameraDevice mCameraDevice;
    private CameraCaptureSession mCaptureSession;

    // used for image capture and analysis
    private ImageReader mImageReader;
    private String mCameraId;

    // Other necessary member variables
    private HandlerThread mBackgroundThread;
    private Handler mBackgroundHandler;
    private Surface mPreviewSurface; // Renamed for clarity
    private TextureView mPreviewView;
    private Size mPreviewSize;

    /**
     * Constructs the Camera2Reader.
     * @param context The application context.
     * @param previewView The TextureView for displaying the camera preview.
     */
    public Camera2Reader(@NonNull Context context, @NonNull TextureView previewView) {
        mContext = context;
        mCameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        this.mPreviewView = previewView;
    }

    /**
     * Starts the camera, which is the main entry point.
     * It sets up the camera, checks for permissions, and opens the camera device.
     */
    public void startCamera(ImageReader.OnImageAvailableListener onImageListener) {
        startBackgroundThread();
        setupCamera(onImageListener);
        if (mCameraId != null) {
            if (ActivityCompat.checkSelfPermission(mContext, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                Log.e(TAG, "Camera permission not granted!");
                return;
            }
            try {
                mCameraManager.openCamera(mCameraId, mStateCallback, mBackgroundHandler);
            } catch (CameraAccessException e) {
                Log.e(TAG, "Failed to open camera.", e);
            }
        }
    }

    public Size getPreviewSize() {
        return mPreviewSize;
    }

    /**
     * Stops the camera and releases all related resources.
     */
    public void stopCamera() {
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
        stopBackgroundThread();
    }


    private static final Size RESOLUTION_1080P = new Size(1920, 1080);
    private static final Size RESOLUTION_720P = new Size(1280, 720);

    public boolean supportsResolution(StreamConfigurationMap cfgMap, Size targetSize) {
        Size[] sizes = cfgMap.getOutputSizes(ImageFormat.JPEG);
        if (sizes == null) return false;

        for (Size size : sizes) {
            if (size.equals(targetSize)) {
                return true;
            }
        }
        return false;
    }

    private String getFormatName(int format) {
        switch (format) {
            case ImageFormat.YUV_420_888:
                return "YUV_420_888";
            case ImageFormat.JPEG:
                return "JPEG";
            case ImageFormat.RGB_565:
                return "RGB_565";
            case ImageFormat.NV21:
                return "NV21";
            case ImageFormat.YUY2:
                return "YUY2";
            case ImageFormat.YUV_422_888:
                return "YUV_422_888";
            case ImageFormat.YUV_444_888:
                return "YUV_444_888";
            case ImageFormat.FLEX_RGB_888:
                return "FLEX_RGB_888";
            case ImageFormat.FLEX_RGBA_8888:
                return "FLEX_RGBA_8888";
            case PixelFormat.RGBA_8888:
                return "RGBA_8888";
            case PixelFormat.RGBX_8888:
                return "RGBX_8888";
            case PixelFormat.RGB_888:
                return "RGB_888";
            default:
                return "UNKNOWN(" + format + ")";
        }
    }

    public Size checkPreviewSupport(StreamConfigurationMap cfgMap) {
        Size previewSize;
        boolean supports1080p = supportsResolution(cfgMap, RESOLUTION_1080P);
        boolean supports720p  = supportsResolution(cfgMap, RESOLUTION_720P);

        // 定义常见的图像格式用于查询
        int[] formats = cfgMap.getOutputFormats();

        // 推荐方式：遍历 getOutputFormats() 返回的格式列表
        Log.d("CameraConfig", "=== Supported Output Formats and Sizes ===");
        for (int format : formats) {
            String formatName = getFormatName(format);
            android.util.Size[] sizes = cfgMap.getOutputSizes(format);

            if (sizes != null && sizes.length > 0) {
                // 排序以便查看（从大到小）
                Arrays.sort(sizes, (a, b) -> Integer.compare(b.getWidth() * b.getHeight(), a.getWidth() * a.getHeight()));

                StringBuilder sizeStr = new StringBuilder();
                for (android.util.Size size : sizes) {
                    sizeStr.append(String.format("%dx%d, ", size.getWidth(), size.getHeight()));
                }
                // 去掉最后的逗号空格
                if (sizeStr.length() > 2) {
                    sizeStr.setLength(sizeStr.length() - 2);
                }

                Log.d("CameraConfig", String.format("Format: %s [%d] → %s",
                        formatName, format, sizeStr.toString()));
            } else {
                Log.d("CameraConfig", String.format("Format: %s [%d] → No supported sizes", formatName, format));
            }
        }
        Log.d(TAG, "Camera Preview supports 1080p: " + supports1080p);
        Log.d(TAG, "Camera Preview supports  720p: " + supports720p);

        // preferred preview size
        if (supports1080p) {
            previewSize = RESOLUTION_1080P;
        } else if (supports720p) {
            previewSize = RESOLUTION_720P;
        } else {
            // Fallback: select the largest available size
            previewSize = Collections.max(
                    Arrays.asList(cfgMap.getOutputSizes(ImageFormat.JPEG)),
                    (size1, size2) -> Long.signum((long) size1.getWidth() * size1.getHeight() - (long) size2.getWidth() * size2.getHeight())
            );
        }

        // update default preview size
        mPreviewView.getSurfaceTexture().setDefaultBufferSize(1280, 720);
        mPreviewSurface = new Surface(mPreviewView.getSurfaceTexture());
        return previewSize;
    }

    public static String getPreferredCameraId(CameraManager cameraManager) {
        String backCameraId = null;
        String firstCameraId = null;

        try {
            // Iterate through all available camera IDs
            for (String cameraId : cameraManager.getCameraIdList()) {
                CameraCharacteristics characteristics = cameraManager.getCameraCharacteristics(cameraId);

                // Record the first camera as fallback
                if (firstCameraId == null) {
                    firstCameraId = cameraId;
                }

                // query available frame-rate range(e.g.[15,30], [24,30], [30,30] etc)
                Range<Integer>[] fpsRange = characteristics.get(CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
                Log.d(TAG, String.format("CameraId=%s Target FPS Ranges=%s", cameraId, Arrays.toString(fpsRange)));

                // Check if this is a back-facing camera
                Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
                if (facing != null && facing == CameraCharacteristics.LENS_FACING_BACK) {
                    backCameraId = cameraId;
                    Log.d(TAG, "Found back-facing camera: " + cameraId);
                    break; // Prefer the first back camera; exit early
                }
            }

            // Return back camera if available
            if (backCameraId != null) {
                Log.d(TAG, "Selected back camera: " + backCameraId);
                return backCameraId;
            }

            // Otherwise, return the first available camera
            if (firstCameraId != null) {
                Log.d(TAG, "No back camera found. Using first available camera: " + firstCameraId);
                return firstCameraId;
            }

            Log.w(TAG, "No camera found on device.");
            return null;

        } catch (Exception e) {
            Log.e(TAG, "Failed to get camera ID", e);
            return null;
        }
    }

    /**
     * Configures the camera by selecting a camera ID and an appropriate preview size.
     */
    private void setupCamera(ImageReader.OnImageAvailableListener onImageListener) {
        try {
            mCameraId = getPreferredCameraId(mCameraManager);
            if (null != mCameraId) {
                CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(mCameraId);
                // enumerateSupportedConfigs(mCameraManager, mCameraId);

                // Get supported stream configurations
                StreamConfigurationMap cfgMap = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                if (null != cfgMap) {
                    // Find a suitable preview size
                    mPreviewSize = checkPreviewSupport(cfgMap);
                    Log.d(TAG, String.format("PreviewSize = {%d x %d}", mPreviewSize.getWidth(), mPreviewSize.getHeight()));

                    // Create an ImageReader to receive preview frames
                    mImageReader = ImageReader.newInstance(mPreviewSize.getWidth(), mPreviewSize.getHeight(),
                            ImageFormat.YUV_420_888, 3);
                    mImageReader.setOnImageAvailableListener(onImageListener, mBackgroundHandler);
                }
            }
        } catch (CameraAccessException e) {
            Log.e(TAG, "Failed to setup camera.", e);
        }
    }

    /**
     * Callback for CameraDevice state changes.
     */
    private final CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            Log.d(TAG, "Camera device opened.");
            mCameraDevice = cameraDevice;
            createCameraPreviewSession();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice cameraDevice) {
            Log.d(TAG, "Camera device disconnected.");
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice cameraDevice, int error) {
            Log.e(TAG, "Camera device error: " + error);
            cameraDevice.close();
            mCameraDevice = null;
        }
    };

    /**
     * Creates a camera preview session to start streaming.
     */
    private void createCameraPreviewSession() {
        try {
            // Create a CaptureRequest.Builder for the preview
            CaptureRequest.Builder builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            builder.addTarget(mPreviewSurface); // Target for the preview Surface
            // builder.addTarget(mImageReader.getSurface()); // Target for the ImageReader Surface

            // config target fps range, e.g. 30fps
            builder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE,  new Range<>(30, 30));

            // Create the CameraCaptureSession
            // Arrays.asList(mPreviewSurface, mImageReader.getSurface())
            mCameraDevice.createCaptureSession(Arrays.asList(mPreviewSurface),
                    new CameraCaptureSession.StateCallback() {
                        @Override
                        public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
                            mCaptureSession = cameraCaptureSession;
                            try {
                                // Start the repeating request to begin the preview stream
                                cameraCaptureSession.setRepeatingRequest(builder.build(), null, mBackgroundHandler);
                            } catch (CameraAccessException e) {
                                Log.e(TAG, "Failed to start camera preview.", e);
                            }
                        }

                        @Override
                        public void onConfigureFailed(@NonNull CameraCaptureSession cameraCaptureSession) {
                            Log.e(TAG, "Failed to configure camera session.");
                        }
                    }, mBackgroundHandler);
        } catch (CameraAccessException e) {
            Log.e(TAG, "Failed to create camera preview session.", e);
        }
    }

    /**
     * Helper method to start the background thread for camera operations.
     */
    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("CameraBackground");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    /**
     * Helper method to stop the background thread.
     */
    private void stopBackgroundThread() {
        if (mBackgroundThread != null) {
            mBackgroundThread.quitSafely();
            try {
                mBackgroundThread.join();
                mBackgroundThread = null;
                mBackgroundHandler = null;
            } catch (InterruptedException e) {
                Log.e(TAG, "Interrupted while stopping background thread.", e);
            }
        }
    }
}