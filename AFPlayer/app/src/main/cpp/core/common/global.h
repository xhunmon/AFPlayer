/**
 * description: 属于播放器中全局的参数，如：播放状态等、解码后的音频视频数据队列（因为多个地方使用同一个） <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/8
 */
#ifndef AFPLAYER_GLOBAL_H
#define AFPLAYER_GLOBAL_H

extern "C" {
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>
};
#include "android_log.h"

#include "macro.h"
#include "safe_queue.h"

class Global {
public:
    Global();

    ~Global();

    Status status;
    SafeQueue<AVPacket *> *audioPktQ = nullptr;
    SafeQueue<AVPacket *> *videoPktQ = nullptr;
    SafeQueue<AVFrame *> *audioFrameQ = nullptr;
    SafeQueue<AVFrame *> *videoFrameQ = nullptr;
    int64_t seekPos = 0;
    JNIEnv *env;

    /**********     音频相关参数      ****************/
    SLuint32 fmtType;//音频值格式类型，如：浮点类型等
    uint32_t sampleRate = 0;
    uint8_t channels = 0;
    uint8_t bytesPerSample = 0;
    uint32_t nbSample = 0;
    uint32_t frameSize = 0;

    /**********     视频相关参数      ****************/
    bool openHardwareDecode = false;//开启硬件解码出现严重掉帧情况，原因未知
    int videoFrameSize = 0;
    int fps = 0;
    int width = 0;
    int height = 0;
    ANativeWindow *window = nullptr;

    double audioClock = 0;//当前音频时钟
    AVRational audioTimeBase = {0,1};
    AVRational videoTimeBase = {0,1};


    void setAllQueueWork(bool work);
};


#endif //AFPLAYER_GLOBAL_H
