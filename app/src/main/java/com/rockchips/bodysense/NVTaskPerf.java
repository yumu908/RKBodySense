package com.rockchips.bodysense;

import android.util.Log;

public class NVTaskPerf {

    private long mTimeStart;
    private long mTimeEnd;
    private int mTaskNum;
    private long mTimeConsumed;
    private long mTimeLast;
    private final long mTimeout;

    private final String TAG = "NVTaskPerf";

    public NVTaskPerf(long timeout) {
        mTimeLast = System.currentTimeMillis();
        mTimeout = (timeout < 100)? 100:timeout;
        reset();
    }

    public void start() {
        mTimeStart = System.currentTimeMillis();
    }

    public void end() {
        mTimeEnd = System.currentTimeMillis();
        mTaskNum++;
        mTimeConsumed += (mTimeEnd - mTimeStart);
    }

    public void reset() {
        mTimeStart = 0;
        mTimeEnd = 0;
        mTaskNum = 0;
        mTimeConsumed = 0;
        mTimeLast = System.currentTimeMillis();
    }

    public float getFps() {
        float fps = 0.0f;
        if ((System.currentTimeMillis() - mTimeLast) > mTimeout) {
            fps = 1.0f* mTaskNum * mTimeout/(System.currentTimeMillis() - mTimeLast);
        }
        return fps;
    }

    public void printStats(String summary) {
        if ((System.currentTimeMillis() - mTimeLast) > mTimeout) {
            mTaskNum = Math.max(mTaskNum, 1);
            Log.e(TAG, String.format("%02d FPS and %dms/Time for %s ", mTaskNum, mTimeConsumed/mTaskNum, summary));
            mTimeLast = System.currentTimeMillis();
            reset();
        }
    }
}
