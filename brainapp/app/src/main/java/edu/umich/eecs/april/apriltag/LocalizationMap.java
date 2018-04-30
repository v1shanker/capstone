package edu.umich.eecs.april.apriltag;

import android.support.annotation.Nullable;

import java.util.HashMap;
import java.util.List;

public class LocalizationMap {

    private static LocalizationMap sInstance = null;
    private static HashMap<Point, Integer> obstructions;
    private static HashMap<Integer, Point> codeToPointMap;


    public static LocalizationMap getInstance() {
        if (sInstance == null) {
            sInstance = new LocalizationMap();
        }
        return sInstance;
    }

    /* This needs to be filled in once we have a map */
    private HashMap<Integer, Point> getCodeToPointMap() {
        return new HashMap<Integer, Point>();
    }

    /* This needs to be filled in once we have a map */
    private HashMap<Point, Integer> getObstructions() {
        return new HashMap<Point, Integer>();
    }

    private LocalizationMap() {
        obstructions = getObstructions();
        codeToPointMap = getCodeToPointMap();
    }

    public Integer getObstruction(Point p) {
        return obstructions.getOrDefault(p, 0);
    }

    public void setObstruction(Point p, Integer v) {
        obstructions.put(p, v);
    }

    public Point getPointLocation(Integer i) {
        return codeToPointMap.getOrDefault(i, null);
    }

}
