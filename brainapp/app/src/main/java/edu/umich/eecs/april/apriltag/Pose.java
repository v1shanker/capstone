package edu.umich.eecs.april.apriltag;

class Pose {
    public double x;
    public double y;

    public double theta;

    public Pose(double xArg, double yArg, double thetaArg) {
        x = xArg;
        y = yArg;
        theta = thetaArg;
    }
}
