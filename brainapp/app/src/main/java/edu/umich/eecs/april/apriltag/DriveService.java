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
import java.util.List;

public class DriveService extends Service {
    public static final String HEIGHT = "Height";
    public static final String WIDTH = "Width";

    private static final String TAG = "DriveService";
    private static final long DELAY_MS = 1000;
    private Handler mHandler = null;
    private SystemState mSystemState = SystemState.getInstance();
    private BodyConnection mBodyConnection = BodyConnection.getInstance();
    private Localization mLocalization = null;

    private long frameCount = 0;

    private static final int STARTUP_COOLDOWN = 10;
    private static final int TURN_COOLDOWN = 30;

    private static final double DISTANCE_THRESHOLD = 0.1;
    private static final double ANGLE_THRESHOLD = Math.PI / 4.0;

    private Pose mPose;

    private Point mPoint;
    private Point mTarget;
    private Point mNavPoint;
    private List<Point> mNavigationPath;

    private enum MovementCommand {
        NONE, TARGET_X, TARGET_Y, TARGET_ANGLE
    }
    private MovementCommand mMovementCommand;
    private double mMovementCommandValue;

    private enum MotorState {
        UNKNOWN, STOP, FORWARD, BACKWARD, RIGHT, LEFT
    }
    private MotorState mMotorState;
    private int mMotorCooldown;

    public void initDrive() {
        mPose = null;

        mMotorState = MotorState.UNKNOWN;
        mMotorCooldown = STARTUP_COOLDOWN;

        mTarget = new Point(Localization.worldToGrid(0.0), Localization.worldToGrid(0.0));
        mNavigationPath = null;

        mMovementCommand = MovementCommand.NONE;

        LocalizationMap m = LocalizationMap.getInstance();
        // harbour table area
//        m.setPointLocation(0, new Pose(0.0, 0.0, 0.0));
//        m.setPointLocation(1, new Pose(0.66, 0.023, 0.02));
//        m.setPointLocation(5, new Pose(0.031, -0.522, -Math.PI+0.01));
//        m.setPointLocation(6, new Pose(0.68, -0.513, 0.0));

        // harbour tv area
//        m.setPointLocation(4, new Pose(0.0, 0.0, 0.0));

        // rowe's lab
        m.setPointLocation(22, new Pose(0.0, 0.0, Math.PI));
        m.setPointLocation(20, new Pose(-0.6, 0.78, -1.58));
        m.setPointLocation(21, new Pose(0.448, 0.844, -1.06));
        m.setPointLocation(23, new Pose(-0.81, 1.8, -1.54));


    }

    public void readLidar() {
        //mBodyConnection.send("LSCAN\n");
    }

    public void updateLocation() {
        List<ApriltagDetection> tags = mSystemState.getDetectedTagList();
        if (tags == null) { return; }
        List<Pose> poseEstimations = new ArrayList<>();
        for (ApriltagDetection tag : tags) {
            Pose p = mLocalization.getPoseFromTag(tag);
            if (p != null) {
                Log.v(TAG, String.format("%d: %f, %f, %f", tag.id, p.x, p.y, p.theta));
                poseEstimations.add(p);
            }
        }

        if (poseEstimations.isEmpty()) {
            Log.v(TAG, "No tags to localize to");
            return;
        }

        double totalX = 0.0;
        double totalY = 0.0;
        double totalSin = 0.0;
        double totalCos = 0.0;
        for (Pose p : poseEstimations) {
            totalX += p.x;
            totalY += p.y;
            totalSin += Math.sin(p.theta);
            totalCos += Math.cos(p.theta);
        }

        mPose = new Pose(totalX / poseEstimations.size(),
                         totalY / poseEstimations.size(),
                         Math.atan2(totalSin, totalCos));

        Log.i(TAG, String.format("Pose: %f, %f, %f", mPose.x, mPose.y, mPose.theta));
    }

    public void updateNavigation() {
        if (mPose == null) { return; }

        int x = Localization.worldToGrid(mPose.x);
        int y = Localization.worldToGrid(mPose.y);
        mPoint = new Point(x, y);

        if (mNavigationPath == null || !mNavigationPath.contains(mPoint)) {
            Log.i(TAG, "Navigation: Recalculating path");
            mNavigationPath = PathPlanning.findPath(mPoint, mTarget);
            mNavPoint = mPoint;
            mMovementCommand = MovementCommand.NONE;
            for (Point p : mNavigationPath) {
                Log.i(TAG, String.format("(%d,%d)", p.getXCoord(), p.getYCoord()));
            }
        }
    }

    private double getAngle(Point from, Point to) {
        if (from.getXCoord() != to.getXCoord()) {
            if (from.getXCoord() > to.getXCoord()) {
                return -Math.PI;
            } else {
                return 0.0;
            }
        } else {
            if (from.getYCoord() > to.getYCoord()) {
                return -(Math.PI / 2.0);
            } else {
                return Math.PI / 2.0;
            }
        }
    }

    private double angleDiff(double x, double y) {
        return Localization.normalizeAngle(x - y);
    }

    public void updateMovementCommand() {
        if (mPose == null || mNavigationPath == null || mNavigationPath.isEmpty()) { return; }

        if (mMovementCommand == MovementCommand.NONE && mNavPoint != mTarget) {
            int i = mNavigationPath.indexOf(mPoint);
            int j = i+1;
            if (j == mNavigationPath.size()) { return; }
            Point next = mNavigationPath.get(j);

            // first check if a turn is needed
            double toGo = getAngle(mNavPoint, next);
            if (Math.abs(angleDiff(toGo, mPose.theta)) > ANGLE_THRESHOLD) {
                Log.i(TAG, String.format("MovementCommand: Turn from %f to %f", mPose.theta, toGo));
                mMovementCommand = MovementCommand.TARGET_ANGLE;
                mMovementCommandValue = toGo;
                return;
            }

            if (mNavPoint.getXCoord() == next.getXCoord()) {
                // keep increasing index as long as x coord doesn't change
                int targetIndex = j;
                while (targetIndex < mNavigationPath.size() - 1 &&
                        mNavPoint.getXCoord() == mNavigationPath.get(targetIndex + 1).getXCoord()) {
                    targetIndex++;
                }
                int targetY = mNavigationPath.get(targetIndex).getYCoord();
                Log.i(TAG, String.format("MovementCommand: Go from y=%d to y=%d",
                        mNavPoint.getYCoord(), targetY));
                mMovementCommand = MovementCommand.TARGET_Y;
                mMovementCommandValue = Localization.gridToWorld(targetY);
            } else {
                // keep increasing index as long as y coord doesn't change
                int targetIndex = j;
                while (targetIndex < mNavigationPath.size() - 1 &&
                        mNavPoint.getYCoord() == mNavigationPath.get(targetIndex + 1).getYCoord()) {
                    targetIndex++;
                }
                int targetX = mNavigationPath.get(targetIndex).getXCoord();
                Log.i(TAG, String.format("MovementCommand: Go from x=%d to x=%d",
                        mNavPoint.getXCoord(), targetX));
                mMovementCommand = MovementCommand.TARGET_X;
                mMovementCommandValue = Localization.gridToWorld(targetX);
            }
        }
    }

    private void stop() {
        if (mMotorState == MotorState.STOP) { return; }
//        Log.d(TAG, "Motor: Stop");
        mMotorState = MotorState.STOP;
        mBodyConnection.send("MSTOP\n");
    }

    private void forward() {
        if (mMotorState == MotorState.FORWARD) { return; }
//        Log.d(TAG, "Motor: Forward");
        mMotorState = MotorState.FORWARD;
        mBodyConnection.send("MFWD\n");
    }

    private void backward() {
        if (mMotorState == MotorState.BACKWARD) { return; }
//        Log.d(TAG, "Motor: Backward");
        mMotorState = MotorState.BACKWARD;
        mBodyConnection.send("MBACK\n");
    }

    private void right() {
        if (mMotorState == MotorState.RIGHT) { return; }
        if (mMotorState != MotorState.STOP) {
            stop();
        }
//        Log.d(TAG, "Motor: Right");
        mMotorState = MotorState.STOP;
        mBodyConnection.send("MRIGHT\n");
        mMotorCooldown = TURN_COOLDOWN;
    }

    private void left() {
        if (mMotorState == MotorState.LEFT) { return; }
        if (mMotorState != MotorState.STOP) {
            stop();
        }
//        Log.d(TAG, "Motor: Left");
        mMotorState = MotorState.STOP;
        mBodyConnection.send("MLEFT\n");
        mMotorCooldown = TURN_COOLDOWN;
    }

    public void updateMotor() {
        if (mPose == null || mNavigationPath == null || mMotorCooldown > 0) { return; }

        switch (mMovementCommand) {
            case NONE:
                if (mMotorState != MotorState.STOP) {
                    stop();
                }
                break;

            case TARGET_X:
                double targetX = mMovementCommandValue;
                double deltaX = targetX - mPose.x;
                if (Math.abs(mPose.theta) > Math.PI / 2.0) { deltaX = -deltaX; }
                if (deltaX > DISTANCE_THRESHOLD) {
                    forward();
                } else if (deltaX < -DISTANCE_THRESHOLD) {
                    backward();
                } else {
                    stop();
                    mMovementCommand = MovementCommand.NONE;
                }
                break;

            case TARGET_Y:
                double targetY = mMovementCommandValue;
                double deltaY = targetY - mPose.y;
                if (mPose.theta < 0) { deltaY = -deltaY; }
                if (deltaY > DISTANCE_THRESHOLD) {
                    forward();
                } else if (deltaY < -DISTANCE_THRESHOLD) {
                    backward();
                } else {
                    stop();
                    mMovementCommand = MovementCommand.NONE;
                }
                break;

            case TARGET_ANGLE:
                double targetTheta = mMovementCommandValue;
                double deltaTheta = Localization.normalizeAngle(targetTheta - mPose.theta);
                if (deltaTheta > ANGLE_THRESHOLD) {
                    left();
                } else if (deltaTheta < -ANGLE_THRESHOLD) {
                    right();
                } else {
                    mMovementCommand = MovementCommand.NONE;
                }
                break;
        }

    }

    Runnable DriveUpdateRunnable = new Runnable() {
        @Override
        public void run() {
            try {
                // do work
                frameCount++;
                if (mMotorCooldown > 0) { mMotorCooldown--; }
                updateLocation();
                if (mBodyConnection.isConnected()) {
                    //readLidar();
                    updateNavigation();
                    updateMovementCommand();
                    updateMotor();
                    mBodyConnection.handleInput();
                } else {
                    Log.d(TAG, "Attempting to connect");
                    mBodyConnection.connect(DriveService.this);
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
