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
        System.loadLibrary("native-lib");
    }

    public native static String getFFVersion();
}
