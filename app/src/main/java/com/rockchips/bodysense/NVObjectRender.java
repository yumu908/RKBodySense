package com.rockchips.bodysense;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.util.Log;
import android.util.Size;
import android.widget.ImageView;

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

public class NVObjectRender {
    private final ImageView mObjectView;
    private Bitmap mViewBitmap = null;
    private Canvas mViewCanvas = null;
    private Paint mLinePaint = null;
    private Paint mTextPaint = null;
    private Size mPreviewSize = null;

    private final Map<Integer, KalmanFilter2D> mKalmanMap = new HashMap<>();
    private int taskTimes = 0;

    public double[] update(int id, double posX, double posY) {
        long timeNow = System.currentTimeMillis();
        KalmanFilter2D kf = mKalmanMap.get(id);
        if (kf == null) {
            kf = new KalmanFilter2D(1.0/50.0f);
            mKalmanMap.put(id, kf);
        }
        double[] newPos;
        if ((posX > 0) && (posY > 0)) {
            newPos = new double[]{posX, posY};
            kf.update(newPos);
        } else {
            newPos = kf.getPosition();
        }

        // check and remove old KalmanFilter
        removeExpired(timeNow, 120);

        return newPos;
    }

    public void removeExpired(long expireMillis, int maxTimes) {
        taskTimes++;
        if (taskTimes < maxTimes) {
            return ;
        }

        long now = System.currentTimeMillis();
        Iterator<Map.Entry<Integer, KalmanFilter2D>> it = mKalmanMap.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry<Integer, KalmanFilter2D> entry = it.next();
            KalmanFilter2D kf = entry.getValue();
            if ((now - kf.getLastUpdateTime()) > expireMillis) {
                System.out.println("Cleaning up expired KalmanFilter for ID: " + entry.getKey());
                it.remove();
            }
        }
    }

    private final String TAG = "NVObjectRender";

    public static int sp2px(float spValue) {
        Resources r = Resources.getSystem();
        final float scale = r.getDisplayMetrics().scaledDensity;
        return (int) (spValue * scale + 0.5f);
    }

    public NVObjectRender(ImageView objectView) {
        mObjectView  = objectView;
        mPreviewSize = new Size(1920, 1080);

        prepare();
    }

    private void prepare() {
        int width  = mObjectView.getWidth();
        int height = mObjectView.getHeight();

        if (width * height < 100) {
            return;
        }
        mViewBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        mViewCanvas = new Canvas(mViewBitmap);

        // create paint to draw lines
        mLinePaint = new Paint();
        mLinePaint.setColor(Color.YELLOW);
        mLinePaint.setStrokeJoin(Paint.Join.ROUND);
        mLinePaint.setStrokeCap(Paint.Cap.ROUND);
        mLinePaint.setStrokeWidth(3);
        mLinePaint.setStyle(Paint.Style.FILL);
        mLinePaint.setTextAlign(Paint.Align.LEFT);
        mLinePaint.setTextSize(sp2px(100));
        mLinePaint.setTypeface(Typeface.SANS_SERIF);
        mLinePaint.setFakeBoldText(false);
    }
    public void drawObjects(Object detObjects, NVDetectType detType) {
        if (null == mViewBitmap) {
            prepare();
        }
        // clear screen.
        mLinePaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
        mViewCanvas.drawPaint(mLinePaint);
        mLinePaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));

        switch (detType) {
          case DETECT_OBJECT:
          case DETECT_HANDS:
          case DETECT_FACE:
          case DETECT_FACE_LANDMARKS:
            drawObjects(detObjects);
            break;
          case DETECT_BODY_LANDMARKS:
            drawBodyLandmarks(detObjects);
            break;
          default:
            break;
        }
    }

    private void drawObjects(Object detObjects) {
        Log.d(TAG, String.format("not implemented for this type!"));
    }

    public void setPreviewSize(Size previewSize) {
        mPreviewSize = previewSize;
    }

    private void drawRotatedRect(Canvas viewCanvas, NVInfoLandmarks.NVRectRotated rect, Paint mLinePaint) {
        double angleRad = Math.toRadians(rect.angle); // 将角度转换为弧度

        // 计算原始顶点位置
        float[] originalX = {rect.cx - rect.width / 2, rect.cx + rect.width / 2, rect.cx - rect.width / 2, rect.cx + rect.width / 2};
        float[] originalY = {rect.cy - rect.height / 2, rect.cy - rect.height / 2, rect.cy + rect.height / 2, rect.cy + rect.height / 2};

        // 应用旋转转换
        float[] rotatedX = new float[4];
        float[] rotatedY = new float[4];
        for (int i = 0; i < 4; i++) {
            rotatedX[i] = (float) (rect.cx + (originalX[i] - rect.cx) * Math.cos(angleRad) - (originalY[i] - rect.cy) * Math.sin(angleRad));
            rotatedY[i] = (float) (rect.cy + (originalX[i] - rect.cx) * Math.sin(angleRad) + (originalY[i] - rect.cy) * Math.cos(angleRad));
        }

        // 绘制矩形边
        viewCanvas.drawLine(rotatedX[0], rotatedY[0], rotatedX[1], rotatedY[1], mLinePaint);
        viewCanvas.drawLine(rotatedX[1], rotatedY[1], rotatedX[3], rotatedY[3], mLinePaint);
        viewCanvas.drawLine(rotatedX[3], rotatedY[3], rotatedX[2], rotatedY[2], mLinePaint);
        viewCanvas.drawLine(rotatedX[2], rotatedY[2], rotatedX[0], rotatedY[0], mLinePaint);
    }

    private void drawBodyLandmarks(Object detObjects) {
        int width  = mObjectView.getWidth();
        int height = mObjectView.getHeight();

        //detect result
        NVInfoLandmarks landmarks = (NVInfoLandmarks)detObjects;
        if ((landmarks != null) && (landmarks.count >0)) {
            //Log.d(TAG, String.format("landmarks.count=%d", landmarks.count));
            for(int idx = 0; idx < landmarks.count; idx++) {
                NVInfoLandmarks.NVInfoLandmark landmark = landmarks.landmarks.get(idx);

                // Log.d(TAG, "RotatedRect=" + landmark.leftHandBox.toString());

                // draw body box
                Rect rect = landmark.box;
                // Log.d(TAG, String.format("ViewRender(%dx%d) BodyRect(%03d %03d %03d %03d)", width, height,
                //        rect.left, rect.top, rect.right, rect.bottom));
                String oldRect = rect.toString();
                double scaleX = (double) width  / 640;
                double scaleY = (double) (height) / 320;
                rect.left  = (int) (rect.left * scaleX);
                rect.right = (int) (rect.right * scaleX);
                rect.top = rect.top - 24;
                rect.top = (int) (rect.top * scaleY);
                rect.bottom = (int) (rect.bottom * scaleY);
                if ((rect.right - rect.left) * (rect.bottom - rect.top) < 256 * 128) {
                    return;
                }
                mViewCanvas.drawRect(rect, mLinePaint);

                // draw left and right hand-box
                landmark.leftHandBox.cx = (int) (landmark.leftHandBox.cx * scaleX);
                landmark.leftHandBox.cy = (int) (landmark.leftHandBox.cy * scaleY);
                landmark.leftHandBox.width  = (int) (landmark.leftHandBox.width * scaleX);
                landmark.leftHandBox.height = (int) (landmark.leftHandBox.height * scaleY);
                drawRotatedRect(mViewCanvas, landmark.leftHandBox, mLinePaint);
                landmark.rightHandBox.cx = (int) (landmark.rightHandBox.cx * scaleX);
                landmark.rightHandBox.cy = (int) (landmark.rightHandBox.cy * scaleY);
                landmark.rightHandBox.width  = (int) (landmark.rightHandBox.width * scaleX);
                landmark.rightHandBox.height = (int) (landmark.rightHandBox.height * scaleY);
                drawRotatedRect(mViewCanvas, landmark.rightHandBox, mLinePaint);

                // draw body id
                float textWidth = mLinePaint.measureText(Integer.toString(landmark.id));
                mViewCanvas.drawText(Integer.toString(landmark.id),
                        (rect.left + rect.right - textWidth)/2, (rect.top+rect.bottom)/2, mLinePaint);

                // draw body mesh
                List<Point> points = landmarks.landmarks.get(idx).points;
                mLinePaint.setStyle(Paint.Style.FILL);
                for (Point point : points) {
                    point.x = (int) (point.x * scaleX);
                    point.y = point.y - 20;
                    point.y = (int) (point.y * scaleY);
                    point.x = Math.max(rect.left, Math.min(point.x, rect.right));
                    point.y = Math.max(rect.top, Math.min(point.y, rect.bottom));
                    mViewCanvas.drawCircle(point.x, point.y, 10, mLinePaint);
                }
                mLinePaint.setStyle(Paint.Style.STROKE);

                NVBodyPose[][] bodyLines = {
                        // keypoint for face
                        {NVBodyPose.NOSE, NVBodyPose.LEFT_EYE},
                        {NVBodyPose.NOSE, NVBodyPose.RIGHT_EYE},
                        {NVBodyPose.LEFT_EYE, NVBodyPose.LEFT_EAR},
                        {NVBodyPose.RIGHT_EYE, NVBodyPose.RIGHT_EAR},
                        // keypoint for upper body
                        {NVBodyPose.LEFT_SHOULDER, NVBodyPose.RIGHT_SHOULDER},
                        {NVBodyPose.LEFT_SHOULDER, NVBodyPose.LEFT_ELBOW},
                        {NVBodyPose.RIGHT_SHOULDER, NVBodyPose.RIGHT_ELBOW},
                        {NVBodyPose.LEFT_ELBOW, NVBodyPose.LEFT_WRIST},
                        {NVBodyPose.RIGHT_ELBOW, NVBodyPose.RIGHT_WRIST},
                        // keypoint for lower body
                        {NVBodyPose.LEFT_HIP, NVBodyPose.RIGHT_HIP},
                        {NVBodyPose.LEFT_HIP, NVBodyPose.LEFT_KNEE},
                        {NVBodyPose.LEFT_KNEE, NVBodyPose.LEFT_ANKLE},
                        {NVBodyPose.RIGHT_HIP, NVBodyPose.RIGHT_KNEE},
                        {NVBodyPose.RIGHT_KNEE, NVBodyPose.RIGHT_ANKLE},
                        // keypoint for Torso
                        {NVBodyPose.LEFT_SHOULDER, NVBodyPose.LEFT_HIP},
                        {NVBodyPose.RIGHT_SHOULDER, NVBodyPose.RIGHT_HIP}
                };

                // draw keypoint for body pose.
                for (NVBodyPose[] line : bodyLines) {
                    Point start = points.get(line[0].ordinal());
                    Point end   = points.get(line[1].ordinal());
                    mViewCanvas.drawLine(start.x, start.y, end.x, end.y, mLinePaint);
                }
            }
        }
        mObjectView.setScaleType(ImageView.ScaleType.FIT_XY);
        mObjectView.setImageBitmap(mViewBitmap);
    }

    // 19 points of body-pose
    private enum NVBodyPose {
        NOSE, LEFT_EYE, RIGHT_EYE, LEFT_EAR, RIGHT_EAR,
        LEFT_SHOULDER, RIGHT_SHOULDER, LEFT_ELBOW, RIGHT_ELBOW,
        LEFT_WRIST, RIGHT_WRIST, LEFT_HIP, RIGHT_HIP,
        LEFT_KNEE, RIGHT_KNEE, LEFT_ANKLE, RIGHT_ANKLE,
        LEFT_HAND, RIGHT_HAND,
    }

    public enum NVDetectType {
        DETECT_OBJECT, DETECT_HANDS, DETECT_FACE,
        DETECT_FACE_LANDMARKS, DETECT_BODY_LANDMARKS, DETECT_TYPE_MAX,
    };
}
