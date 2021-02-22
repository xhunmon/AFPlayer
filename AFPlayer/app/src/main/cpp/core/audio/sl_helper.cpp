/**
 * description: OpenSL ES播放器帮助类 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/9
 */
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "../common/android_log.h"

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