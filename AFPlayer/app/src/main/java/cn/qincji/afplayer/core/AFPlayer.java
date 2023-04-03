package cn.qincji.afplayer.core;

import android.view.Surface;

import cn.qincji.afplayer.core.callback.OnAFCallback;

/**
 * description:   <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/7
 */
public class AFPlayer {
    static {
        System.loadLibrary("avformat");
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("x264.161");
        System.loadLibrary("fdk-aac");
        System.loadLibrary("ssl.3");
        System.loadLibrary("crypto.3");
        System.loadLibrary("native-lib");
    }

    public AFPlayer(){
        native_create();
    }

    public void setStatusCallback(OnAFCallback onAFCallback){
        native_setStatusCallback(onAFCallback);
    }

    public void setDataSource(String srcFile, Surface surface){
        native_setDataSource(srcFile,surface);
    }

    public void start() {
        native_start();
    }

    public void release() {
        native_release();
    }

    public int getHeight(){
        return native_getHeight();
    }

    public int getWidth(){
        return native_getWidth();
    }

    public long getDuration(){
        return native_getDuration();
    }

    private native void native_create();
    private native void native_setStatusCallback(OnAFCallback onAFCallback);

    private native void native_setDataSource(String src, Surface surface);

    private native int native_getHeight();

    private native int native_getWidth();

    private native long native_getDuration();

    private native int native_start();

    private native int native_release();
}
