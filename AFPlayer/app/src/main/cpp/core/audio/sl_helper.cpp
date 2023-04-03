/**
 * description: OpenSL ES播放器帮助类 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/9
 */
extern "C" {
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
};

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "../common/android_log.h"
#include "../common/const.h"

//static const SLEnvironmentalReverbSettings reverbSettings =
//        SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
static const SLEnvironmentalReverbSettings reverbSettings =
        SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;

static bool checkError(SLresult *r, const char *what) {
    if (*r != SL_RESULT_SUCCESS) {
        LOGE("Error %s (result %lu)", what, (long unsigned int) r);
        LOGE("DISABLING SOUND!");
        return true;
    }
    (void) *r;
    return false;
}

static SLuint32 getBitsPerSample(uint8_t bytesPerSample) {
    switch (bytesPerSample) {
        case 1:
            return SL_PCMSAMPLEFORMAT_FIXED_8;
        case 3:
            return SL_PCMSAMPLEFORMAT_FIXED_24;
        case 4:
            return SL_PCMSAMPLEFORMAT_FIXED_32;
        case 2:
        default:
            return SL_PCMSAMPLEFORMAT_FIXED_16;
    }
}

static SLuint32 getChannelMask(uint8_t channels) {
    if (channels == 1) {
        return SL_SPEAKER_FRONT_CENTER;
    } else {
        return SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }
}

static SLuint32 getChannelLayout(uint8_t channels) {
    if (channels == 1) {
        return AV_CH_FRONT_LEFT;
    } else {
        return AV_CH_LAYOUT_STEREO;
    }
}


static int getCurrentSampleRateForSl(int sampleRate) {
    int rate = 0;
    switch (sampleRate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}


static bool isNeedSwr(int sample_rate, AVSampleFormat sample_fmt, int &outSampleRate,
                      AVSampleFormat **outSampleFmt) {
    bool needSampleRate;
    bool needSampleFmt;
    switch (sample_rate * 1000) {//需要根据OpenSL ES所支持的来判断是否需要重采样。
        case SL_SAMPLINGRATE_8:
        case SL_SAMPLINGRATE_11_025:
        case SL_SAMPLINGRATE_12:
        case SL_SAMPLINGRATE_16:
        case SL_SAMPLINGRATE_22_05:
        case SL_SAMPLINGRATE_24:
        case SL_SAMPLINGRATE_32:
        case SL_SAMPLINGRATE_44_1:
        case SL_SAMPLINGRATE_48:
        case SL_SAMPLINGRATE_64:
        case SL_SAMPLINGRATE_88_2:
        case SL_SAMPLINGRATE_96:
        case SL_SAMPLINGRATE_192:
            needSampleRate = false;
            outSampleRate = sample_rate;
            break;
        default:
            LOGE("OpenSL ES not support the %d sample_rate.", sample_rate);
            needSampleRate = true;
            outSampleRate = DEFAULT_AUDIO_SAMPLE_RATE;
            break;
    }


    //OpenSL ES 1.1支持浮点的值
    switch (sample_fmt) {
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            needSampleFmt = false;
            *outSampleFmt = &sample_fmt;
//           *outFmtType = SL_ANDROID_PCM_REPRESENTATION_FLOAT;
            break;
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S16P:
        case AV_SAMPLE_FMT_S32P:
        case AV_SAMPLE_FMT_S64:
        case AV_SAMPLE_FMT_S64P:
            needSampleFmt = false;
            *outSampleFmt = &sample_fmt;
//            *outFmtType = SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT;
            break;
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_U8P:
            needSampleFmt = false;
            *outSampleFmt = &sample_fmt;
//            *outFmtType = SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT;
            break;
        default:
            LOGE("OpenSL ES not support the %d sample_fmt.", sample_fmt);
            needSampleFmt = true;
            **outSampleFmt = DEFAULT_AUDIO_FORMAT;
//            *outFmtType = SL_ANDROID_PCM_REPRESENTATION_FLOAT;
            break;
    }

    return needSampleRate || needSampleFmt;
//    return true;//需要重采样
}