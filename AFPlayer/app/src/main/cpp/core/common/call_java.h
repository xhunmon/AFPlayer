/**
 * description: 回调给java的处理类 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/7
 */


#ifndef AFPLAYER_CALL_JAVA_H
#define AFPLAYER_CALL_JAVA_H

#include "macro.h"
#include <jni.h>


class CallJava {
public:
    CallJava(JavaVM *vm, JNIEnv *env, jobject instace);

    ~CallJava();

    //回调java
    void onError(ThreadType threadType, int code, const char *error);

    void onPrepare(ThreadType threadType);

    void onPlaying(ThreadType threadType, int64_t curDuration, int64_t totalDuration);

private:
    JavaVM *vm;
    JNIEnv *env;
    jobject instance;
    jmethodID onPlayingId;
    jmethodID onErrorId;
    jmethodID onPreparedId;
};

#endif //AFPLAYER_CALL_JAVA_H
