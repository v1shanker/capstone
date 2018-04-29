package edu.umich.eecs.april.apriltag;

import static java.lang.Math.abs;
import static java.lang.Math.atan;

public class ApriltagDetection {

    // THESE CONSTANTS SHOULD BE CHANGED
    // THESE ARE PHONE, CAMERA, AND TAG SPECIFIC
    private static double REAL_HEIGHT_MM = 30.0;
    private static double IMAGE_HEIGHT_PIXELS = 1080.0;
    private static double SENSOR_HEIGHT_MM = 6.17;
    private static double FOCAL_LENGTH_MM = 26.6;

    public static double getTagHeight(ApriltagDetection tag) {
        double max, min;
        max = tag.p[1];
        min = tag.p[1];
        for (int i = 1; i < tag.p.length; i += 2) {
            if (tag.p[i] > max) {
                max = tag.p[i];
            }
            if (tag.p[i] < min) {
                min = tag.p[i];
            }
        }

        return abs(max-min);
    }

    public static double getTagWidth(ApriltagDetection tag) {
        double max, min;
        max = tag.p[0];
        min = tag.p[0];
        for (int i = 0; i < tag.p.length; i += 2) {
            if (tag.p[i] > max) {
                max = tag.p[i];
            }
            if (tag.p[i] < min) {
                min = tag.p[i];
            }
        }

        return abs(max-min);
    }

    public static double getDistanceFromTag(ApriltagDetection tag ) {
        return (FOCAL_LENGTH_MM * REAL_HEIGHT_MM * IMAGE_HEIGHT_PIXELS) / ( getTagHeight(tag) * SENSOR_HEIGHT_MM);
    }

    public static double getAngleFromTag(ApriltagDetection tag) {
        return atan(getTagHeight(tag)/getTagWidth(tag));
    }

    // The decoded ID of the tag
    public int id;

    // How many error bits were corrected? Note: accepting large numbers of
    // corrected errors leads to greatly increased false positive rates.
    // NOTE: As of this implementation, the detector cannot detect tags with
    // a hamming distance greater than 2.
    public int hamming;

    // The center of the detection in image pixel coordinates.
    public double[] c = new double[2];

    // The corners of the tag in image pixel coordinates. These always
    // wrap counter-clock wise around the tag.
    // Flattened to [x0 y0 x1 y1 ...] for JNI convenience
    public double[] p = new double[8];

    // Homography matrix info extracted for convenience
    public int ncols;
    public int nrows;
    public double[] H_data = new double[9];

}
