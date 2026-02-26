package com.rockchips.bodysense;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.media.Image;
import android.media.ImageReader;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.TextureView;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.JSONObject;
import com.rockchips.bodysense.utils.Camera2Reader;

import java.text.DecimalFormat;
import java.util.ArrayList;

public class BodyPoseActivity extends AppCompatActivity {
    private static final int HANDLE_SHOW_FPS = 1;
    private static final int HANDLE_SHOW_RESULT = 2;

    private TextureView mSurfaceView;
    private Camera2Reader mCameraReader;

    private TextView mFpsNum1;
    private TextView mFpsNum2;
    private TextView mFpsNum3;
    private TextView mFpsNum4;

    private NVTaskPerf mTaskPerf;
    private NVTaskRockx mTaskRockx;
    private NVObjectRender mObjectRender;

    private Float mFrameRate;
    private int  mFrameCount;
    private long mFrameTime;

    final String TAG = "RockxBodyPose";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // hiddend navigation
        View decorView = getWindow().getDecorView();
        int uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_FULLSCREEN;
        decorView.setSystemUiVisibility(uiOptions);

        setContentView(R.layout.activity_body_pose);
        mFpsNum1 = findViewById(R.id.fps_num1);
        mFpsNum2 = findViewById(R.id.fps_num2);
        mFpsNum3 = findViewById(R.id.fps_num3);
        mFpsNum4 = findViewById(R.id.fps_num4);

        // check app permissions
        checkAppPermissions();

        KalmanFilter2D.unit_test(null);

        // initCamera();
        mSurfaceView = findViewById(R.id.camera_preview);
        mSurfaceView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
            @Override
            public void onSurfaceTextureAvailable(@NonNull SurfaceTexture surfaceTexture, int i, int i1) {
                if (mCameraReader== null) {
                    mCameraReader = new Camera2Reader(BodyPoseActivity.this, mSurfaceView);
                }
                mCameraReader.startCamera(mOnImageListener);
                mFrameTime  = System.currentTimeMillis();
                mFrameCount = 0;
                mFrameRate  = 30.0f;
            }

            @Override
            public void onSurfaceTextureSizeChanged(@NonNull SurfaceTexture surfaceTexture, int i, int i1) {

            }

            @Override
            public boolean onSurfaceTextureDestroyed(@NonNull SurfaceTexture surfaceTexture) {
                if (mCameraReader != null) {
                    mCameraReader.stopCamera();
                }
                return true;
            }

            @Override
            public void onSurfaceTextureUpdated(@NonNull SurfaceTexture surfaceTexture) {

            }
        });

        this.setTitle(getString(R.string.app_name) + " v" + BuildConfig.VERSION_NAME);

        mTaskRockx = new NVTaskRockx(null, null);
        mTaskRockx.onCreate();
        mTaskPerf = new NVTaskPerf(1000);

        mTaskRockx.setTaskListener(new NVTaskRockx.OnTaskListener() {
            @Override
            public void onListener(String jsonString) {
                mFrameCount++;
                long currentTime = System.currentTimeMillis();
                if (currentTime - mFrameTime >= 1000) {
                    mFrameRate = (float) (mFrameCount / ((currentTime - mFrameTime) / 1000));
                    mFrameCount = 0;
                    mFrameTime = currentTime;
                    updateMainUI(HANDLE_SHOW_FPS, mFrameRate);
                }
                // parse and render detected objects
                parseJsonObjects(jsonString);
            }
        });
    }

    private final ImageReader.OnImageAvailableListener mOnImageListener = reader -> {
        try (Image image = reader.acquireNextImage()) {

        } catch (Exception e) {
            Log.e(TAG, "Failed to acquire image.", e);
        }
    };

    private static final int CAMERA_PERMISSION_REQUEST_CODE = 101;
    private static final int STORAGE_PERMISSION_REQUEST_CODE = 102;

    private void checkAppPermissions() {
        // check and request permission.CAMERA
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, CAMERA_PERMISSION_REQUEST_CODE);
        } else {
            // permission.CAMERA is granted, perform camera-related operations
            System.out.println("Camera permission granted!");
        }

        // check and request permission.READ_EXTERNAL_STORAGE
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, STORAGE_PERMISSION_REQUEST_CODE);
        } else {
            // permission.READ_EXTERNAL_STORAGE is granted, perform storage-related operations
            System.out.println("Storage permission granted!");
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (grantResults.length > 0) {
            Log.d(TAG, String.format("permissions=%s granted=%d", permissions[0], grantResults[0]));
            if (requestCode == CAMERA_PERMISSION_REQUEST_CODE && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                if (mSurfaceView != null && mSurfaceView.isAvailable()) {
                    if (mCameraReader == null) {
                        mCameraReader = new Camera2Reader(BodyPoseActivity.this, mSurfaceView);
                    }
                    try {
                        mCameraReader.startCamera(mOnImageListener);
                    } catch (Exception e) {
                        Log.e(TAG, "startCamera failed after permission granted", e);
                    }
                }
            }
        }
    }

    @Override
    protected void onDestroy() {
        if (mCameraReader != null) {
            mCameraReader.stopCamera();
            mCameraReader = null;
        }
        if (mTaskRockx != null) {
            mTaskRockx.setTaskListener(null);
        }
        mHandler.removeCallbacksAndMessages(null);
        super.onDestroy();
    }

    @Override
    public void onPause() {
        if (mCameraReader != null) {
            mCameraReader.stopCamera();
            mCameraReader = null;
        }
        if (mTaskRockx != null) {
            mTaskRockx.setTaskListener(null);
        }
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        mObjectRender = new NVObjectRender(this.findViewById(R.id.canvasView));
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED && mSurfaceView != null) {
            if (mSurfaceView.isAvailable()) {
                if (mCameraReader == null) {
                    mCameraReader = new Camera2Reader(BodyPoseActivity.this, mSurfaceView);
                }
                try {
                    mCameraReader.startCamera(mOnImageListener);
                } catch (Exception e) {
                    Log.e(TAG, "startCamera failed on resume", e);
                }
                mFrameTime  = System.currentTimeMillis();
                mFrameCount = 0;
                mFrameRate  = 30.0f;
            }
        }
        if (mTaskRockx != null) {
            mTaskRockx.setTaskListener(new NVTaskRockx.OnTaskListener() {
                @Override
                public void onListener(String jsonString) {
                    mFrameCount++;
                    long currentTime = System.currentTimeMillis();
                    if (currentTime - mFrameTime >= 1000) {
                        mFrameRate = (float) (mFrameCount / ((currentTime - mFrameTime) / 1000));
                        mFrameCount = 0;
                        mFrameTime = currentTime;
                        updateMainUI(HANDLE_SHOW_FPS, mFrameRate);
                    }
                    parseJsonObjects(jsonString);
                }
            });
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    private void updateMainUI(int what, Object data) {
        Message msg = mHandler.obtainMessage();
        msg.what = what;
        msg.obj = data;
        mHandler.sendMessage(msg);
    }

    final Handler mHandler = new Handler(Looper.getMainLooper())
    {
        public void handleMessage(Message msg)
        {
            if (msg.what == HANDLE_SHOW_FPS) {
                float fps = (float) msg.obj;
                DecimalFormat decimalFormat = new DecimalFormat("00.00");
                String fpsStr = decimalFormat.format(fps);
                mFpsNum1.setText(String.valueOf(fpsStr.charAt(0)));
                mFpsNum2.setText(String.valueOf(fpsStr.charAt(1)));
                mFpsNum3.setText(String.valueOf(fpsStr.charAt(3)));
                mFpsNum4.setText(String.valueOf(fpsStr.charAt(4)));
            } else if (msg.what == HANDLE_SHOW_RESULT) {
                if ((null != mObjectRender) && (null != mCameraReader)) {
                    mTaskPerf.start();
                    mObjectRender.setPreviewSize(mCameraReader.getPreviewSize());
                    mObjectRender.drawObjects(msg.obj, NVObjectRender.NVDetectType.DETECT_BODY_LANDMARKS);
                    mTaskPerf.end();
                }
            }
            if (mTaskPerf != null) {
                mTaskPerf.printStats("Render");
            }
        }
    };

    private int parseJsonObjects(String jsonLandmarks) {
        // Log.d(TAG,  jsonLandmarks);
        JSONObject root = null;
        try {
            root = JSONObject.parseObject(jsonLandmarks);
        } catch (JSONException e) {
            e.printStackTrace();
            return -1;
        }
        NVInfoLandmarks landmarks = new NVInfoLandmarks();

        // parse metadata for NVInfoLandmarks
        landmarks.isTracked = root.getIntValue("isTracked");
        landmarks.count     = root.getIntValue("count");
        // Log.e(TAG, String.format("=============landmarks.count=%d", landmarks.count));
        JSONObject imageSizeObj = root.getJSONObject("imageSize");
        landmarks.imageSize = new NVInfoLandmarks.NVSize();
        landmarks.imageSize.width = imageSizeObj.getIntValue("width");
        landmarks.imageSize.height = imageSizeObj.getIntValue("height");

        // parse landmark array.
        JSONArray landmarkArray = root.getJSONArray("landmarks");
        // Log.e(TAG, String.format("=============landmarks.size()=%d", landmarkArray.size()));
        landmarks.landmarks = new ArrayList<NVInfoLandmarks.NVInfoLandmark>();
        for (int idx = 0; idx < landmarkArray.size(); idx++) {
            // allocate NVInfoLandmark
            NVInfoLandmarks.NVInfoLandmark landmark = new NVInfoLandmarks.NVInfoLandmark();

            // parse metadata for NVInfoLandmark
            JSONObject landmarkObj = landmarkArray.getJSONObject(idx);
            landmark.id = landmarkObj.getIntValue("id");
            landmark.score = landmarkObj.getFloatValue("score");

            JSONObject aHand = landmarkObj.getJSONObject("leftHandBox");
            landmark.leftHandBox  = new NVInfoLandmarks.NVRectRotated();
            landmark.leftHandBox.cx = aHand.getInteger("cx");
            landmark.leftHandBox.cy = aHand.getInteger("cy");
            landmark.leftHandBox.width  = aHand.getInteger("width");
            landmark.leftHandBox.height = aHand.getInteger("height");
            landmark.leftHandBox.angle  = aHand.getInteger("angle");

            JSONObject bHand = landmarkObj.getJSONObject("rightHandBox");
            landmark.rightHandBox = new NVInfoLandmarks.NVRectRotated();
            landmark.rightHandBox.cx = bHand.getInteger("cx");
            landmark.rightHandBox.cy = bHand.getInteger("cy");
            landmark.rightHandBox.width  = bHand.getInteger("width");
            landmark.rightHandBox.height = bHand.getInteger("height");
            landmark.rightHandBox.angle  = bHand.getInteger("angle");

            // parse landmark.box
            JSONObject boxObj = landmarkObj.getJSONObject("box");
            landmark.box = new Rect(0,0,0,0);
            landmark.box.left   = boxObj.getIntValue("left");
            landmark.box.top    = boxObj.getIntValue("top");
            landmark.box.right  = boxObj.getIntValue("right");
            landmark.box.bottom = boxObj.getIntValue("bottom");

            JSONArray points = landmarkObj.getJSONArray("points");
            landmark.points = new ArrayList<Point>();
            for (int j = 0; j < points.size(); j++) {
                JSONArray pointItem = points.getJSONArray(j);
                int x = pointItem.getIntValue(0);
                int y = pointItem.getIntValue(1);
                landmark.points.add(new Point(x,y));
            }

            // add to landmarks.landmarks
            landmarks.landmarks.add(landmark);
        }
        updateMainUI(HANDLE_SHOW_RESULT, landmarks);
        return 0;
    }
}

