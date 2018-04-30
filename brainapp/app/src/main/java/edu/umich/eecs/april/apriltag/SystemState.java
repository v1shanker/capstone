package edu.umich.eecs.april.apriltag;

import android.support.annotation.Nullable;

import java.util.List;

class SystemState {

    private static SystemState sInstance = null;

    private List<ApriltagDetection> mDetectedTags;
    private LocalizationMap mLocalizationmap;

    public static SystemState getInstance() {
        if (sInstance == null) {
            sInstance = new SystemState();
        }
        return sInstance;
    }

    private SystemState() {
        mDetectedTags = null;
        mLocalizationmap = LocalizationMap.getInstance();
    }

    @Nullable
    public synchronized List<ApriltagDetection> getDetectedTagList() {
        return mDetectedTags;
    }

    public synchronized void setDetectedTagList(List<ApriltagDetection> detectedTagList) {
        mDetectedTags = detectedTagList;
    }
}
