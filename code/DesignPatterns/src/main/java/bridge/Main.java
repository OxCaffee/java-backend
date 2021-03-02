package main.java.bridge;

public class Main {

    public static void main(String[] args) {
        //白色涂笔
        Color white = new WhiteColor();
        //圆形
        Shape circle = new CircleShape(white);
        //绘图
        circle.draw();
    }
}
