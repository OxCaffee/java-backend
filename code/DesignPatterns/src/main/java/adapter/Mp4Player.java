package main.java.adapter;

public class Mp4Player implements AdvancedMediaPlayer{

    @Override
    public void playVlc(String fileName) {
        //MP4播放器什么也不做
    }

    @Override
    public void playMp4(String fileName) {
        System.out.println("正在播放MP4，文件名称: " + fileName);
    }
}
