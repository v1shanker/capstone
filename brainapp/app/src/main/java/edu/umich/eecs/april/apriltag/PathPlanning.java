package edu.umich.eecs.april.apriltag;

import java.util.ArrayList;

public class PathPlanning {

    private static ArrayList<Point> checkHorizontalPathFirst(Point start, Point end, LocalizationMap lmap) {
        ArrayList<Point> path = new ArrayList<Point>();

        // check horizontal
        int y_coor = start.getYCoord();
        for(int x_i = start.getXCoord(); x_i <= end.getXCoord(); x_i++) {
            Point p = new Point(x_i, y_coor);
            if (lmap.getObstruction(p) != 0) {
                return null;
            }
            path.add(p);
        }

        // check vertical
        int x_coor = end.getXCoord();
        for(int y_i = start.getYCoord(); y_i <= end.getYCoord(); y_i++) {
            Point p = new Point(x_coor, y_i);
            if (lmap.getObstruction(p) != 0) {
                return null;
            }
            if (y_i != start.getYCoord()) {
                path.add(p);
            }
        }

        return path;
    }

    private static ArrayList<Point> checkVerticalPathFirst(Point start, Point end, LocalizationMap lmap) {
        ArrayList<Point> path = new ArrayList<Point>();

        // check vertical
        int x_coor = start.getXCoord();
        for(int y_i = start.getYCoord(); y_i <= end.getYCoord(); y_i++) {
            Point p = new Point(x_coor, y_i);
            if (lmap.getObstruction(p) != 0) {
                return null;
            }
        }

        // check horizontal
        int y_coor = end.getYCoord();
        for(int x_i = start.getXCoord(); x_i <= end.getXCoord(); x_i++) {
            Point p = new Point(x_i, y_coor);
            if (lmap.getObstruction(p) != 0) {
                return null;
            }
            if (x_i != start.getXCoord()) {
                path.add(p);
            }
        }

        return path;
    }

    public static ArrayList<Point> findPath(Point start, Point end) {
        LocalizationMap lmap = LocalizationMap.getInstance();
        ArrayList<Point> path;

        path = checkHorizontalPathFirst(start, end, lmap);
        if (path != null) {
            return path;
        }
        path = checkVerticalPathFirst(start, end, lmap);
        if (path != null) {
            return path;
        }

        return null;
    }

    // Only use the static utilities
    // There is not reason to have an instance of this object
    private PathPlanning() {

    }


}
