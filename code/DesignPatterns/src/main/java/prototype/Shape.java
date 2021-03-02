package main.java.prototype;

public abstract class Shape implements Cloneable{

    private String id;
    protected String type;

    abstract void draw();

    @Override
    protected Object clone(){
        Object cloned = null;
        try {
            cloned = super.clone();
        }catch (CloneNotSupportedException e){
            e.printStackTrace();
        }
        return cloned;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }
}
