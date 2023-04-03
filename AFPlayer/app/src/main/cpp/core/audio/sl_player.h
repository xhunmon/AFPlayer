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
#include "../ffmpeg/ff_decoder.h"

class SLPlayer {
public:
    SLObjectItf engineObj = nullptr;
    SLEngineItf engineItf = nullptr;
    SLObjectItf outputMixObj = nullptr;
    SLObjectItf playerObj = nullptr;
    SLPlayItf playItf = nullptr;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = nullptr;
    SLVolumeItf volumeItf = nullptr;
    SLAndroidSimpleBufferQueueItf bufferQueue = nullptr;
public:
    pthread_t pidPlay;
    Global *global = nullptr;
    AVCodecContext *audioDecCtx = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *pkt = nullptr;
    int data_size = 0;
    uint8_t *buffer = nullptr;


    SLPlayer(AVCodecContext *audioDecCtx);
    ~SLPlayer();

    int prepare(Global *g);

    int initOpenSl();

    int decodeAudio();

    void play();
    void start();

private:
    SLresult createEngine();
};


#endif //AFPLAYER_SL_PLAYER_H
