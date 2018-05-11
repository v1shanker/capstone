package edu.umich.eecs.april.apriltag;

import android.support.annotation.Nullable;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

class Localization {
    private final static String TAG = "Localization";

    private final static double FOCAL_LENGTH_METERS = 0.028;

    private final static double SCREEN_DPI = 534.0; // change based on phone
    private final static double INCHES_PER_METER = 39.37;

    private final static double PIXELS_PER_METER = SCREEN_DPI * INCHES_PER_METER;

    private final static double CALIBRATION_FACTOR = 0.82;

    private final static double CAMERA_HEIGHT_METERS = 0.2;
    private final static double CAMERA_OFFSET_FORWARD_METERS = -0.0285;
    private final static double CAMERA_OFFSET_RIGHT_METERS = 0.062;

    private double camCenterX;
    private double camCenterY;

    private LocalizationMap mMap = LocalizationMap.getInstance();

    public Localization(int width, int height) {
        camCenterX = width / 2.0;
        camCenterY = height / 2.0;
    }

    private double scalePxToWorld(double pxOffset, double heightMeters) {
        double scalingFactor = heightMeters / FOCAL_LENGTH_METERS;
        double offsetMeters = pxOffset / PIXELS_PER_METER;
        return offsetMeters * scalingFactor / CALIBRATION_FACTOR;
    }

    public static double normalizeAngle(double angle) {
        while (angle >= Math.PI) {
            angle -= 2 * Math.PI;
        }
        while (angle < -Math.PI) {
            angle += 2 * Math.PI;
        }
        return angle;
    }

    // estimate position and orientation within the world based on observing this tag
    public Pose getPoseFromTag(ApriltagDetection tag) {
        // Reference for tag's position
        Pose tagPose = mMap.getPointLocation(tag.id);
        if (tagPose == null) {
            Log.d(TAG, "Unrecognized tag");
            return null;
        }

        double tagHeight = 2.54 - CAMERA_HEIGHT_METERS;
        double tagPosX = tagPose.x;
        double tagPosY = tagPose.y;
        double phi = tagPose.theta;

        double tagCenterX = tag.c[0];
        double tagCenterY = tag.c[1];

        // calculate vector RT (robot to tag) first in rectangular pixel-sized grid
        // lower y = forward, lower x = right
        double tagOffsetFwd = camCenterY - tagCenterY + CAMERA_OFFSET_FORWARD_METERS;
        double tagOffsetRight = camCenterX - tagCenterX + CAMERA_OFFSET_RIGHT_METERS;

        // transform RT to polar meter-sized coordinate system
        double radiusPx = Math.sqrt(tagOffsetRight * tagOffsetRight +
                                    tagOffsetFwd * tagOffsetFwd);
        double radiusRT = scalePxToWorld(radiusPx, tagHeight);
        double thetaRT = Math.atan2(tagOffsetFwd, tagOffsetRight);

        // T hat is the tag's orientation within the robot-centric coordinate system
        // p lists points in the order bottom left, bottom right, top right, top left
        double botLeftX = tag.p[0];
        double botLeftY = tag.p[1];
        double topLeftX = tag.p[6];
        double topLeftY = tag.p[7];
        double vecFwd = botLeftY - topLeftY;
        double vecRight = botLeftX - topLeftX;
        double thetaTHat = Math.atan2(vecFwd, vecRight);

        // Differences in angles will be the same in robot-centric coords as in world coords
        double theta = thetaTHat - thetaRT; // to convert RT to world rect coords
        double robotAngleRelToTag = (Math.PI / 2) - thetaTHat;

        double x = tagPosX - radiusRT * Math.cos(phi - theta);
        double y = tagPosY - radiusRT * Math.sin(phi - theta);
        double thetaRes = normalizeAngle(phi + robotAngleRelToTag);

        return new Pose(x, y, thetaRes);
    }
}
