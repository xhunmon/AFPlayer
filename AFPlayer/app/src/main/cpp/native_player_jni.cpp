#include <jni.h>
#include <string>
#include "core/player_main.h"

extern "C" {
#include "libavcodec/jni.h"
}
//@link：https://zhuanlan.zhihu.com/p/547250316

Player *player = nullptr;
JavaVM *javaVm = nullptr;

extern "C"
JNIEXPORT
jint JNI_OnLoad(JavaVM *vm, void *res) {
    av_jni_set_java_vm(vm, nullptr);
    // 返回jni版本
    javaVm = vm;
    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT void JNICALL
Java_cn_qincji_afplayer_core_AFPlayer_native_1create(JNIEnv *env, jobject thiz) {
    player = new Player();
}

extern "C"
JNIEXPORT void JNICALL
Java_cn_qincji_afplayer_core_AFPlayer_native_1setStatusCallback(JNIEnv *env, jobject thiz,
                                                                jobject on_afcallback) {
    if (player) {
        player->setJavaStatucCallback(javaVm, env, &on_afcallback);
    } else {
        LOGW("播放器已被销毁！！");
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_cn_qincji_afplayer_core_AFPlayer_native_1setDataSource(JNIEnv *env, jobject thiz,
                                                            jstring src, jobject surface) {
    if (player) {
        const char *pSrc = env->GetStringUTFChars(src, nullptr);
        ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
        player->setDataSource(pSrc, window, env);
//        env->ReleaseStringUTFChars(src, pSrc);
    } else {
        LOGW("播放器已被销毁！！");
    }

}

extern "C"
JNIEXPORT jint JNICALL
Java_cn_qincji_afplayer_core_AFPlayer_native_1getHeight(JNIEnv *env, jobject thiz) {
    if (player) {
        return player->getHeight();
    } else {
        LOGW("播放器已被销毁！！");
        return 0;
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_cn_qincji_afplayer_core_AFPlayer_native_1getWidth(JNIEnv *env, jobject thiz) {
    if (player) {
        return player->getWidth();
    } else {
        LOGW("播放器已被销毁！！");
        return 0;
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_cn_qincji_afplayer_core_AFPlayer_native_1getDuration(JNIEnv *env, jobject thiz) {
    if (player) {
        return player->getDuration();
    } else {
        LOGW("播放器已被销毁！！");
        return 0;
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_cn_qincji_afplayer_core_AFPlayer_native_1start(JNIEnv *env, jobject thiz) {
    if (player) {
        return player->start();
    } else {
        LOGW("播放器已被销毁！！");
        return 0;
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_cn_qincji_afplayer_core_AFPlayer_native_1release(JNIEnv *env, jobject thiz) {
    if (player) {
        delete player;
        player = nullptr;
        return 0;
    } else {
        LOGW("播放器已被销毁！！");
        return 0;
    }
}