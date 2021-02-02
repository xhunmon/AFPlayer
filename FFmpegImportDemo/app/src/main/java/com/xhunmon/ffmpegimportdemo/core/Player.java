package com.xhunmon.ffmpegimportdemo.core;

/**
 * description:   <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/1/24
 */
public class Player {
    static {
        //动态库的操作 avformat avcodec avfilter avutil swresample swscale
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
    public native static String getFFVersion();
}
