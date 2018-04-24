package edu.umich.eecs.april.apriltag;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

import java.util.List;
import java.util.Locale;

public class DriveService extends Service {
    private static final String TAG = "DriveService";
    private static final long DELAY_MS = 1000;
    private Handler mHandler = null;
    private SystemState mSystemState = SystemState.getInstance();
    private BodyConnection mBodyConnection = BodyConnection.getInstance();
    private Context mContext = null;

    private enum MotorState {
        STOP, FORWARD
    }
    private MotorState mMotorState;

    public void initDrive() {
        mContext = this;
    }

    public void printTags() {
        List<ApriltagDetection> tags = mSystemState.getDetectedTagList();
        if (tags != null && !tags.isEmpty()) {
            StringBuilder sb = new StringBuilder();
            sb.append("AprilTags: ");
            for (ApriltagDetection tag : tags) {
                int id = tag.id;
                double dist = ApriltagDetection.getDistanceFromTag(tag);
                double angle = ApriltagDetection.getAngleFromTag(tag);

                sb.append(String.format(Locale.getDefault(), "(%d, %f, %f) ", id, dist, angle));
            }
            Log.d(TAG, sb.toString());
        } else {
            Log.d(TAG, "No AprilTags in range!");
        }
    }

    public void readLidar() {}

    public void updateMotor() {

    }

    Runnable DriveUpdateRunnable = new Runnable() {
        @Override
        public void run() {
            try {
                // do work
                printTags();
                if (mBodyConnection.isConnected()) {
                    readLidar();
                    updateMotor();
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
        // Create separate thread, since service runs in foreground by default
        HandlerThread thread = new HandlerThread("DriveThread");
        thread.start();

        Looper looper = thread.getLooper();
        mHandler = new Handler(looper);

        // setup
        initDrive();

        // start main loop
        mHandler.post(DriveUpdateRunnable);
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
