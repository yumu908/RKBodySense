package com.rockchips.bodysense;

import android.graphics.Point;
import android.graphics.Rect;

import java.util.List;

public class NVInfoLandmarks {
    public int count; // 人体数量
    public List<NVInfoLandmark> landmarks; // 人体关键点
    public NVSize imageSize; // 图像尺寸
    public int isTracked; // 是否跟踪

    // 内部类：表示单个人体的关键点信息
    public static class NVInfoLandmark {
        public int id; // 目标id
        public float score; // 置信度
        public Rect box;  // 人形框
        public NVRectRotated leftHandBox;  // 左手框
        public NVRectRotated rightHandBox; // 右手框
        public float leftHandScore;  // 左手置信度
        public float rightHandScore; // 右手置信度
        public int count; // 关键点数量
        public List<Point> points; // 关键点坐标
        public List<Float> scores;   // 关键点置信度
    }

    // 内部类：表示旋转矩形区域
    public static class NVRectRotated {
        public float cx;
        public float cy;
        public float width;
        public float height;
        public float angle;
    }

    // 内部类：表示点坐标
    public static class NVPoint {
        public float x;
        public float y;

        public NVPoint(float x, float y) {
            this.x = x;
            this.y = y;
        }
    }

    // 内部类：表示图像大小
    public static class NVSize {
        public int width;
        public int height;
    }
}