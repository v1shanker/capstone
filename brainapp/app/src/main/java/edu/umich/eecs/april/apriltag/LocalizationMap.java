package edu.umich.eecs.april.apriltag;

import android.support.annotation.Nullable;

import java.util.HashMap;
import java.util.List;

public class LocalizationMap {

    private static LocalizationMap sInstance = null;
    private static HashMap<Point, Integer> obstructions;
    private static HashMap<Integer, Pose> codeToPointMap;


    public static LocalizationMap getInstance() {
        if (sInstance == null) {
            sInstance = new LocalizationMap();
        }
        return sInstance;
    }

    /* This needs to be filled in once we have a map */
    private HashMap<Integer, Pose> getCodeToPointMap() {
        return new HashMap<Integer, Pose>();
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

    public Pose getPointLocation(Integer i) {
        return codeToPointMap.getOrDefault(i, null);
    }

    public void setPointLocation(Integer i, Pose p) { codeToPointMap.put(i, p); }
}
