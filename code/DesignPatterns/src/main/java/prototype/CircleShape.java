package main.java.prototype;

public class CircleShape extends Shape{

    public CircleShape(){
        type = "圆形";
    }

    @Override
    void draw() {
        System.out.println("绘制圆形");
    }
}
