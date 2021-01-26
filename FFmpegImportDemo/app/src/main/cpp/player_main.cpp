#include <jni.h>
#include <string>

#include <android/log.h>

extern "C" {
#include <libavutil/avutil.h>
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_xhunmon_ffmpegimportdemo_core_Player_getFFVersion(JNIEnv *env, jclass type) {
    return env->NewStringUTF(av_version_info());
}