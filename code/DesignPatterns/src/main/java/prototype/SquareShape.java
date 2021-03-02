package main.java.prototype;

public class SquareShape extends Shape{

    public SquareShape(){
        type = "方形";
    }

    @Override
    void draw() {
        System.out.println("绘制方形");
    }
}
