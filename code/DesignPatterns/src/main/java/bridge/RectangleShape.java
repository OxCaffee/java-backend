package main.java.bridge;

public class RectangleShape extends Shape{

    public RectangleShape(Color color) {
        super(color);
    }

    @Override
    public void draw() {
        color.paint("正方形");
    }
}
