package edu.umich.eecs.april.apriltag;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

public class DriveService extends Service {
    public static final String HEIGHT = "Height";
    public static final String WIDTH = "Width";

    private static final String TAG = "DriveService";
    private static final long DELAY_MS = 1000;
    private Handler mHandler = null;
    private SystemState mSystemState = SystemState.getInstance();
    private BodyConnection mBodyConnection = BodyConnection.getInstance();
    private Context mContext = null;
    private Localization mLocalization = null;
    private long frameCount = 0;

    private double mPosX;
    private double mPosY;
    private double mTheta;

    private enum MotorState {
        UNKNOWN, STOP, FORWARD, BACKWARD
    }
    private MotorState mMotorState;

    public void initDrive() {
        mContext = this;
        mMotorState = MotorState.UNKNOWN;

        LocalizationMap m = LocalizationMap.getInstance();
        m.setPointLocation(1, new Pose(0.6, 0.02, 0.02));
        m.setPointLocation(0, new Pose(0.0, 0.0, 0.0));
//        m.setPointLocation(2, new Pose(0.0, 0.0, 0.0));
        m.setPointLocation(4, new Pose(0.0, 0.0, 0.0));
    }

    public void printTags() {
        List<ApriltagDetection> tags = mSystemState.getDetectedTagList();
        if (tags != null) {
            Log.d(TAG, String.format("Detected %d tags.", tags.size()));
            for (ApriltagDetection tag : tags) {
                Pose p = mLocalization.getPoseFromTag(tag);
                if (p == null) continue;

                Log.d(TAG, String.format("id: %d, p: (%f, %f), t: %f", tag.id, p.x, p.y, p.theta));
            }
        } else {
            Log.d(TAG, "No AprilTags in range!");
        }
    }

    public void readLidar() {
        //mBodyConnection.send("LSCAN\n");
    }

    public void updateLocation() {
        List<ApriltagDetection> tags = mSystemState.getDetectedTagList();
        if (tags == null || tags.isEmpty()) { return; }
        List<Pose> poseEstimations = new ArrayList<>();
        for (ApriltagDetection tag : tags) {
            Pose p = mLocalization.getPoseFromTag(tag);
            if (p != null) {
                poseEstimations.add(p);
            }
        }
        double totalX = 0.0;
        double totalY = 0.0;
        for (Pose p : poseEstimations) {
            totalX += p.x;
            totalY += p.y;
        }

        mPosX = totalX / poseEstimations.size();
        mPosY = totalY / poseEstimations.size();
        mTheta = poseEstimations.get(0).theta;
    }

    public void updateMotor() {
        Log.d(TAG, String.format("%f", mPosX));
        if (mPosX > 0.05 && mMotorState != MotorState.FORWARD) {
            Log.d(TAG, "FORWARD");
            mMotorState = MotorState.FORWARD;
            mBodyConnection.send("MFWD\n");
        } else if (mPosX < -0.05 && mMotorState != MotorState.BACKWARD) {
            Log.d(TAG, "BACK");
            mMotorState = MotorState.BACKWARD;
            mBodyConnection.send("MBACK\n");
        } else if (-0.04 < mPosX && mPosX < 0.04 && mMotorState != MotorState.STOP) {
            Log.d(TAG, "STOP");
            mMotorState = MotorState.STOP;
            mBodyConnection.send("MSTOP\n");
        }
    }

    Runnable DriveUpdateRunnable = new Runnable() {
        @Override
        public void run() {
            try {
                // do work
                frameCount++;
                printTags();
                if (mBodyConnection.isConnected()) {
                    //readLidar();
                    updateLocation();
                    updateMotor();
                    mBodyConnection.handleInput();
                } else {
                    Log.d(TAG, "Attempting to connect");
                    mBodyConnection.connect(mContext);
                }
            } finally {
                mHandler.postDelayed(this, DELAY_MS);
            }
        }
    };

    @Override
    public void onCreate() {

    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int width = intent.getIntExtra(WIDTH, 640);
        int height = intent.getIntExtra(HEIGHT, 480);

        mLocalization = new Localization(width, height);

        // Create separate thread, since service runs in foreground by default
        HandlerThread thread = new HandlerThread("DriveThread");
        thread.start();

        Looper looper = thread.getLooper();
        mHandler = new Handler(looper);

        // setup
        initDrive();

        // start main loop
        mHandler.post(DriveUpdateRunnable);

        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        mHandler.removeCallbacks(DriveUpdateRunnable);
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }
}
