package com.xhunmon.ffmpegmediacodecdemo.core;

/**
 * description:   <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/1
 */
public class Player {
    static {
        //动态库的操作
//        System.loadLibrary("avformat");
//        System.loadLibrary("avcodec");
//        System.loadLibrary("avfilter");
//        System.loadLibrary("avutil");
//        System.loadLibrary("swresample");
//        System.loadLibrary("swscale");
        System.loadLibrary("native-lib");
    }

    public native String getFFVersion();
    public native void decode_file(String _inPath, String _outPath);
}
