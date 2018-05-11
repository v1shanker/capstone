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
    private Localization mLocalization = null;

    public void initDrive() {
        LocalizationMap m = LocalizationMap.getInstance();
        m.setPointLocation(0, new Pose(0.0, 0.0, 0.0));
        m.setPointLocation(1, new Pose(0.61, 0.023, 0.02));
        m.setPointLocation(5, new Pose(0.031, -0.472, -Math.PI+0.01));
        m.setPointLocation(6, new Pose(0.63, -0.463, 0.0));
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

    Runnable DriveUpdateRunnable = new Runnable() {
        @Override
        public void run() {
            try {
                printTags();
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
