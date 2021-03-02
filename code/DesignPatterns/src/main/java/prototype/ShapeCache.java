package main.java.prototype;

import java.util.Hashtable;

/**
 * 缓存Shape
 */
public class ShapeCache {

    private static Hashtable<String, Shape> cache
            = new Hashtable<>();

    public static Shape getShape(String shapeId){
        Shape cachedShape = cache.get(shapeId);
        return (Shape) cachedShape.clone();
    }

    public static void loadCache(){
        CircleShape circleShape = new CircleShape();
        circleShape.setId("1");
        cache.put(circleShape.getId(), circleShape);

        SquareShape squareShape = new SquareShape();
        squareShape.setId("2");
        cache.put(squareShape.getId(), squareShape);
    }
}
