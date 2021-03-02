package main.java.adapter;

public class VlcPlayer implements AdvancedMediaPlayer{

    @Override
    public void playVlc(String fileName) {
        System.out.println("正在播放VLC, 文件名称: " + fileName);
    }

    @Override
    public void playMp4(String fileName) {
        //VLC播放器什么也不做
    }
}
