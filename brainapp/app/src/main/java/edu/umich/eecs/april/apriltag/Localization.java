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

    private double camCenterX;
    private double camCenterY;

    public Localization(int width, int height) {
        camCenterX = width / 2.0;
        camCenterY = height / 2.0;
    }

    private double scalePxToWorld(double pxOffset, double heightMeters) {
        double scalingFactor = heightMeters / FOCAL_LENGTH_METERS;
        double offsetMeters = pxOffset / PIXELS_PER_METER;
        return offsetMeters * scalingFactor;
    }

    private double normalizeAngle(double angle) {
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
        double tagCenterX = tag.c[0];
        double tagCenterY = tag.c[1];

        // calculate vector RT (robot to tag) first in rectangular pixel-sized grid
        double tagOffsetFwd = camCenterX - tagCenterX; // lower x = forward
        double tagOffsetRight = camCenterY - tagCenterY; // lower y = right

        // transform RT to polar meter-sized coordinate system
        double tagHeight = 1.785; // TODO look this up based on records of tag's location
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
        double vecFwd = botLeftX - topLeftX;
        double vecRight = botLeftY - topLeftY;
        double thetaTHat = Math.atan2(vecFwd, vecRight);

        // Differences in angles will be the same in robot-centric coords as in world coords
        double theta = thetaTHat - thetaRT; // to convert RT to world rect coords
        double robotAngleRelToTag = (Math.PI / 2.0) - thetaTHat;

        // Reference for tag's position
        // TODO actually look this up
        double tagPosX = 0.0;
        double tagPosY = 0.0;
        double phi = 0.0;

        Pose res = new Pose();
        res.x = tagPosX - radiusRT * Math.cos(phi - theta);
        res.y = tagPosY - radiusRT * Math.sin(phi - theta);
        res.theta = normalizeAngle(phi + robotAngleRelToTag);

        return res;
    }

    public void update(List<ApriltagDetection> tags) {
        List<Pose> poses = new ArrayList<>();

        for (ApriltagDetection tag : tags) {
            poses.add(getPoseFromTag(tag));
        }
    }


}
