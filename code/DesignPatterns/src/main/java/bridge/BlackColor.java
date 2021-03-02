package main.java.bridge;

public class BlackColor implements Color{

    @Override
    public void paint(String shape) {
        System.out.println("黑色的" + shape);
    }
}
