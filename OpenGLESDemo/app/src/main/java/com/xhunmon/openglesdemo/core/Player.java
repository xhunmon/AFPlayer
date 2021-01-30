package com.xhunmon.openglesdemo.core;

import android.view.Surface;

/**
 * description:   <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/1/30
 */
public class Player {
    static {
        System.loadLibrary("native-lib");
    }


    public native void play_video(Surface surface, String path);

}
