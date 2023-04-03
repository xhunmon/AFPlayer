/**
 * description: 回调给java的处理类 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/7
 */

#include "call_java.h"
#include "macro.h"

CallJava::CallJava(JavaVM *vm, JNIEnv *env, jobject instace) {
    this->vm = vm;
    //如果在主线程 回调
    this->env = env;
    // 一旦涉及到jobject 跨方法 跨线程 就需要创建全局引用
    this->instance = env->NewGlobalRef(instace);

    jclass clazz = env->GetObjectClass(instace);
    onPreparedId = env->GetMethodID(clazz, "onPrepared", "()V");
    onErrorId = env->GetMethodID(clazz, "onError", "(ILjava/lang/String;)V");
    onPlayingId = env->GetMethodID(clazz, "onPlaying", "(JJ)V");
}

CallJava::~CallJava() {
    env->DeleteGlobalRef(instance);
}

void CallJava::onError(ThreadType threadType, int code, const char *error) {
    if (threadType == Thread_main) {//主线程
        env->CallVoidMethod(instance, onErrorId, code, env->NewStringUTF(error));
    } else {//子线程
        JNIEnv *my_env;
        //获得属于我这一个线程的jnienv
        vm->AttachCurrentThread(&my_env, nullptr);
        my_env->CallVoidMethod(instance, onErrorId, code, env->NewStringUTF(error));
        vm->DetachCurrentThread();
    }
}

void CallJava::onPrepare(ThreadType threadType) {
    if (threadType == Thread_main) {
        env->CallVoidMethod(instance, onPreparedId);
    } else {
        JNIEnv *my_env;
        vm->AttachCurrentThread(&my_env, nullptr);
        my_env->CallVoidMethod(instance, onPreparedId);
        vm->DetachCurrentThread();
    }
}

void CallJava::onPlaying(ThreadType threadType, int64_t curDuration, int64_t totalDuration) {
    if (threadType == Thread_main) {//主线程
        env->CallVoidMethod(instance, onPlayingId, curDuration, totalDuration);
    } else {//子线程
        JNIEnv *my_env;
        //获得属于我这一个线程的jnienv
        vm->AttachCurrentThread(&my_env, nullptr);
        my_env->CallVoidMethod(instance, onPlayingId, curDuration, totalDuration);
        vm->DetachCurrentThread();
    }
}