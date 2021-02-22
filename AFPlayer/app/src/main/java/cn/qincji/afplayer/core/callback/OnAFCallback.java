package cn.qincji.afplayer.core.callback;

/**
 * description: 播放器的状态回调  <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/7
 */
public interface OnAFCallback {

    /**
     * 调用{#setDataSource(...)}之后
     */
    void onInitialized();

    /**
     * 当进入该状态后：上层可以进行一些播放器的参数设置，如：音量。
     */
    void onPrepared();

    void onStarted();

    void onPaused();

    void onStopped();

    void onSeek(long curDuration);

    void onPlaying(long curDuration);

    void onError(int code, String msg);

    void onReleased();
}
