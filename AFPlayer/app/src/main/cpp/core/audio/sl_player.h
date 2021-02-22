/**
 * description: 使用OpenSL ES进行声音播放 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/9
 */
#ifndef AFPLAYER_SL_PLAYER_H
#define AFPLAYER_SL_PLAYER_H

#include "sl_helper.cpp"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "../common/global.h"

class SLPlayer {
private:
    SLObjectItf engineObj = nullptr;
    SLEngineItf engineItf = nullptr;
    SLObjectItf outputMixObj = nullptr;
    SLObjectItf playerObj = nullptr;
    SLPlayItf playItf = nullptr;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = nullptr;
    SLVolumeItf volumeItf = nullptr;
    pthread_t playPid;
    FILE *inFile;
    SLAndroidSimpleBufferQueueItf bufferQueue = nullptr;
public:
    Global *global = nullptr;


    ~SLPlayer();

    int prepare(Global *g);

    int start();

    int32_t getFrameData(void **buffer, uint8_t *out_buffer) const;

private:
    SLresult createEngine();
};


#endif //AFPLAYER_SL_PLAYER_H
