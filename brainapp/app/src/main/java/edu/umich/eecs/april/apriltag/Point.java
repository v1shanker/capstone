package edu.umich.eecs.april.apriltag;

import java.util.Objects;

public class Point {

    private final int x;
    private final int y;

    public Point(int x_c, int y_c) {
        x = x_c;
        y = y_c;
    }

    public int get_x_coor() {
        return x;
    }

    public int get_y_coor() {
        return y;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Point point = (Point) o;
        return x == point.x &&
                y == point.y;
    }

    @Override
    public int hashCode() {
        return Objects.hash(x, y);
    }
}
