package edu.umich.eecs.april.apriltag;

import android.support.annotation.Nullable;

import java.util.List;

class SystemState {

    private static SystemState sInstance = null;

    private List<ApriltagDetection> mDetectedTags;

    public static SystemState getInstance() {
        if (sInstance == null) {
            sInstance = new SystemState();
        }
        return sInstance;
    }

    private SystemState() {
        mDetectedTags = null;
    }

    @Nullable
    public synchronized List<ApriltagDetection> getDetectedTagList() {
        return mDetectedTags;
    }

    public synchronized void setDetectedTagList(List<ApriltagDetection> detectedTagList) {
        mDetectedTags = detectedTagList;
    }
}
