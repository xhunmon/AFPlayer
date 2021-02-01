#include <jni.h>
#include <string>
extern "C"{
#include "libavcodec/jni.h"
}
#include "core/hw_mediacodec.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_xhunmon_ffmpegmediacodecdemo_core_Player_getFFVersion(JNIEnv *env, jobject type) {
    return env->NewStringUTF(av_version_info());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xhunmon_ffmpegmediacodecdemo_core_Player_decode_1file(JNIEnv *env, jobject thiz, jstring in_path,
                                                           jstring out_path) {
    const char *inPath = env->GetStringUTFChars(in_path, NULL);
    const char *outPath = env->GetStringUTFChars(out_path, NULL);
    new HWMediaCodec(inPath, outPath);
    env->ReleaseStringUTFChars(in_path, inPath);
    env->ReleaseStringUTFChars(out_path, outPath);
}

extern "C"
JNIEXPORT
jint JNI_OnLoad(JavaVM *vm, void *res) {
    av_jni_set_java_vm(vm, 0);
    // 返回jni版本
    return JNI_VERSION_1_4;
}