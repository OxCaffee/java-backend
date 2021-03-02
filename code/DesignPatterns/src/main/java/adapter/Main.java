package main.java.adapter;

public class Main {

    public static void main(String[] args) {
        RealPlayer player = new RealPlayer();

        player.play("mp3", "mp3文件");
        player.play("mp4", "mp4文件");
        player.play("vlc", "vlc文件");
        player.play("avi", "avi文件");
    }
}
