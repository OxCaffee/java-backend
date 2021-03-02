package main.java.adapter;

public class RealPlayer implements MediaPlayer{

    MediaAdapter mediaAdapter;

    @Override
    public void play(String audioType, String fileName) {
        //播放mp3音乐文件的内置支持
        if(audioType.equalsIgnoreCase("mp3")){
            System.out.println("正在播放mp3，文件名称: " + fileName);
        }else if(audioType.equalsIgnoreCase("vlc")
            || audioType.equalsIgnoreCase("mp4")){
            mediaAdapter = new MediaAdapter(audioType);
            mediaAdapter.play(audioType, fileName);
        }else{
            System.out.println("不合法的播放种类: " + audioType);
        }
    }
}
