package edu.umich.eecs.april.apriltag;

import android.support.annotation.Nullable;
import android.util.Log;

import java.util.Arrays;

class Localization {
    private final static String TAG = "Localization";

    @Nullable
    private static double[] crossProd(double[] u, double[] v) {
        if (u.length != 3 || v.length != 3) return null;

        double[] w = new double[3];

        w[0] = u[1] * v[2] - v[1] * u[2];
        w[1] = u[2] * v[0] - v[2] * u[0];
        v[2] = u[0] * v[1] - v[0] * u[1];

        return w;
    }

    /***
     * https://team.inria.fr/lagadic/camera_localization/tutorial-pose-dlt-planar-opencv.html
     * @param nrows rows in homography matrix
     * @param ncols cols in homography matrix
     * @param H values of homography matrix
     * @return Pose estimation, or null if not possible
     */
    @Nullable
    public static Pose getPoseFromHomography(int nrows, int ncols, double[] H) {
        if (nrows != 3 || ncols != 3 || H.length != nrows * ncols) return null;

        Log.d(TAG, Arrays.toString(H));

        // normalize H so that ||c0|| = 1
        double norm = Math.sqrt(H[0] * H[0] + H[3] * H[3] + H[6] * H[6]);
        for (int i = 0; i < 9; i++) {
            H[i] /= norm;
        }

        Log.d(TAG, Arrays.toString(H));

        // cols 0 and 1 of rot matrix are cols 0 and 1 of the normalized homography
        double[] c0 = new double[3];
        c0[0] = H[0]; c0[1] = H[3]; c0[2] = H[6];
        double[] c1 = new double[3];
        c1[0] = H[1]; c1[1] = H[4]; c1[2] = H[7];
        // row 2 of the rotation matrix is c0 x c1
        double[] c2 = crossProd(c0, c1);
        if (c2 == null) return null;

        // combine cols into rotation matrix
        double[] r = new double[9];
        for (int row = 0; row < 3; row++) {
            int starti = row*3;
            r[starti] = c0[row];
            r[starti+1] = c1[row];
            r[starti+2] = c2[row];
        }

        // translation vector is c2 of the normalized homography
        double[] t = new double[3];
        t[0] = H[2]; t[1] = H[5]; t[2] = H[8];

        Pose res = new Pose();
        res.tVec = t;
        res.rRows = 3;
        res.rCols = 3;
        res.rMat = r;

        return res;
    }

}
