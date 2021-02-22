/**
 * description: 控制日志输出，如果闭关打印需要在根 CMakelists.txt设置宏：
 * set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")  <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/7
 */
#ifndef AFPLAYER_ANDROID_LOG_H
#define AFPLAYER_ANDROID_LOG_H
#include <android/log.h>


#if !defined(NDEBUG)

#if !defined(LOG_TAG)
#define LOG_TAG "Native_AFPlayer"
#endif

#define LOGV(...) \
  __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) \
  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) \
  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) \
  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) \
  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGF(...) \
  __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, __VA_ARGS__)

#else

#define LOGV(...)
#define LOGD(...)
#define LOGI(...)
#define LOGW(...)
#define LOGE(...)
#define LOGF(...)
#endif

#endif //AFPLAYER_ANDROID_LOG_H
