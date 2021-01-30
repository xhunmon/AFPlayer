/**
 * description:   <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/1/30
 */
#ifndef PULLPLAYER_VIDEO_COMMON_H
#define PULLPLAYER_VIDEO_COMMON_H

extern "C" {
#include <EGL/egl.h>
//#include <GLES2/gl2.h>
#include <GLES3/gl32.h>
#include <jni.h>
#include <errno.h>
#include <android/sensor.h>
#include <android/log.h>
#include <unistd.h>
#include <stdlib.h>
#include <android/native_window_jni.h>
}

#define DEBUG_TAG "EndlessTunnel:Native"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, DEBUG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, DEBUG_TAG, __VA_ARGS__))
#define ABORT_GAME { LOGE("*** GAME ABORTING."); *((volatile char*)0) = 'a'; }
#define DEBUG_BLIP LOGD("[ BLIP ]: %s:%d", __FILE__, __LINE__)

#define MY_ASSERT(cond) { if (!(cond)) { LOGE("ASSERTION FAILED: %s", #cond); \
   ABORT_GAME; } }

#define BUFFER_OFFSET(i) ((char*)NULL + (i))


#endif //PULLPLAYER_VIDEO_COMMON_H
