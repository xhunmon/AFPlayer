#include <jni.h>
#include <string>

#include "core/audio_player.h"

#include <android/log.h>


extern "C"
JNIEXPORT void JNICALL
Java_com_xhunmon_openslesdemo_core_Player_play_1audio(JNIEnv *env, jobject thiz,
                                                      jstring _filename, jint sampleRate_,
                                                      jint channels_, jint bitPerChannel_) {
    __android_log_print(ANDROID_LOG_DEBUG, "audio_play", "start audio play");
    const char *filename = env->GetStringUTFChars(_filename, NULL);
    AudioPlayer *audioPlayer = new AudioPlayer(filename, sampleRate_, channels_, bitPerChannel_);

    env->ReleaseStringUTFChars(_filename, filename);
    __android_log_print(ANDROID_LOG_DEBUG, "audio_play", "end audio play");
}