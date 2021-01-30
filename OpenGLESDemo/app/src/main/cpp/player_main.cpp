#include <jni.h>
#include <string>

#include "core/video_player.h"

#include <android/log.h>

extern "C"
JNIEXPORT void JNICALL
Java_com_xhunmon_openglesdemo_core_Player_play_1video(JNIEnv *env, jobject thiz,
                                                      jobject _surface,
                                                      jstring _filename) {
    const char *filename = env->GetStringUTFChars(_filename, NULL);
    ANativeWindow *window = ANativeWindow_fromSurface(env, _surface);
    VideoPlayer *pPlayer = new VideoPlayer(window, filename);

    env->ReleaseStringUTFChars(_filename, filename);
}
