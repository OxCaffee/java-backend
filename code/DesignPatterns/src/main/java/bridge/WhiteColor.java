package main.java.bridge;

public class WhiteColor implements Color{

    @Override
    public void paint(String shape) {
        System.out.println("白色的" + shape);
    }
}
