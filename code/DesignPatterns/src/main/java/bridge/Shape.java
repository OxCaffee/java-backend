package main.java.bridge;

/**
 * 解耦，通过持有Color对象
 */
public abstract class Shape {

    Color color;

    public Shape(Color color) {
        this.color = color;
    }

    public abstract void draw();
}
