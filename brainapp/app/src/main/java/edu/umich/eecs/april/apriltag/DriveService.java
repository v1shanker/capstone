package edu.umich.eecs.april.apriltag;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.Nullable;

public class DriveService extends Service {
    private static final long DELAY_MS = 200;
    private Handler mHandler;

    Runnable DriveUpdateRunnable = new Runnable() {
        @Override
        public void run() {
            try {
                // do work
                assert true;
                System.out.println("running!");
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
