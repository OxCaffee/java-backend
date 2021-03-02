package main.java.bridge;

public class CircleShape extends Shape{

    public CircleShape(Color color) {
        super(color);
    }

    @Override
    public void draw() {
        color.paint("圆形");
    }
}
