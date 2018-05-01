package edu.umich.eecs.april.apriltag;

import android.support.annotation.Nullable;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

class Localization {
    private final static String TAG = "Localization";

    private final static double FOCAL_LENGTH_METERS = 0.00338;

    private final static double SCREEN_DPI=534.0; // change based on phone
    private final static double INCHES_PER_METER = 39.37;

    private final static double PIXELS_PER_METER = SCREEN_DPI * INCHES_PER_METER;

    private double camCenterX;
    private double camCenterY;

    public Localization(int height, int width) {
        camCenterX = width / 2.0;
        camCenterY = height / 2.0;
    }

    private double scalePxToWorld(double pxOffset, double heightMeters) {
        double scalingFactor = heightMeters / FOCAL_LENGTH_METERS;
        double offsetMeters = pxOffset / PIXELS_PER_METER;
        return offsetMeters * scalingFactor;
    }

    // get translation and rotation of the robot relative to coordinate system induced by the tag
    private Pose tagRelativePose(ApriltagDetection tag) {

        double tagCenterX = tag.c[0];
        double tagCenterY = tag.c[1];

        // distance and angle from robot to tag
        double tagOffsetX = tagCenterX - camCenterX;
        double tagOffsetY = camCenterY - tagCenterY;
        double radialDist = Math.sqrt(tagOffsetX * tagOffsetX + tagOffsetY * tagOffsetY);
        double dist = scalePxToWorld(radialDist, 1.0);
        double theta = Math.atan2(tagOffsetY, tagOffsetX); // radians ccw from right

        // get tag's orientation
        double tagVecLeft = tag.p[0] - tag.p[2];
        double tagVecFwd = tag.p[1] - tag.p[3];
        double phi = Math.atan2(tagVecLeft, tagVecFwd); // ccw from straight forward

        double angleFromTagToRobot = theta - phi + Math.PI;

        Pose res = new Pose();
        res.x = dist * Math.cos(angleFromTagToRobot);
        res.y = dist * Math.sin(angleFromTagToRobot);
        res.theta = (Math.PI / 2) - phi;

        return res;
    }

    public void update(List<ApriltagDetection> tags) {
        List<Pose> poses = new ArrayList<>();

        for (ApriltagDetection tag : tags) {
            poses.add(tagRelativePose(tag));
        }
    }


}
