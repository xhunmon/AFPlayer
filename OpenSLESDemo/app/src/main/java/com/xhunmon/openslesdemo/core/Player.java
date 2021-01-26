package com.xhunmon.openslesdemo.core;

/**
 * description:   <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/1/26
 */
public class Player {
    static {
        System.loadLibrary("native-lib");
    }

    public native void play_audio(String filename, int sampleRate, int channels, int bitPerChannel);
}
