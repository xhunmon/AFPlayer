/**
 * description: 整个底层播放器入口，对接 native_player_jni.cpp 中所有接口参数 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/7
 */
#ifndef AFPLAYER_PLAYER_MAIN_H
#define AFPLAYER_PLAYER_MAIN_H

#include <android/native_window_jni.h>
#include "common/android_log.h"
#include "common/call_java.h"
#include "ffmpeg/ff_decoder.h"
#include "user_player.h"
#include "common/global.h"
#include "audio/sl_player.h"
#include "video/gl_player.h"
#include <jni.h>

class Player : public UserPlayer {
    CallJava *callJava = nullptr;
    FFDecoder *ffDecoder = nullptr;
    SLPlayer *slPlayer = nullptr;
    GLPlayer *glPlayer = nullptr;
    Global *global;
public:
    void setJavaStatucCallback(JavaVM *javaVm, JNIEnv *env, jobject *onAFCallback);

    virtual void setDataSource(const char *src, ANativeWindow *window, JNIEnv *env);

    virtual long getCurrentPosition();

    virtual long getDuration();

    virtual unsigned int getHeight();

    virtual unsigned int getWidth();

    virtual int start();

    Player();

    ~Player();

    void initPlayerGlobal(ANativeWindow *window, JNIEnv *env);
};


#endif //AFPLAYER_PLAYER_MAIN_H
