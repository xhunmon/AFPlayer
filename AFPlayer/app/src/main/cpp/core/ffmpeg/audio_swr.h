/**
 * description: 使用ffmpeg对音频数据进行重采样 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/10
 */

#ifndef AFPLAYER_AUDIO_SWR_H
#define AFPLAYER_AUDIO_SWR_H


extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
};

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "../common/global.h"
#include "../common/const.h"

class AudioSwr {
private:
    AVCodecContext *decCtx = nullptr;
    Global *global = nullptr;
    SwrContext *swrCtx = nullptr;
public:
    int outSampleRate = 0;
    AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_NONE;
    int nbSample = 1024;
    SLuint32 outFmtType = 0;//OpenSL ES播放值的类型，通用
public:
    AudioSwr(AVCodecContext *decCtx, Global *global);

    ~AudioSwr();

    bool isNeedSwr();

    bool initSwr();

    void resampleAudio(AVFrame **frame);
};


#endif //AFPLAYER_AUDIO_SWR_H
