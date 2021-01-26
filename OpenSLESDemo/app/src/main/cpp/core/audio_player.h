/**
 * description:   <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/1/26
 */

#ifndef PULLPLAYER_AUDIO_PLAYER_H
#define PULLPLAYER_AUDIO_PLAYER_H

#include <cstdio>
#include "audio_common.h"

class AudioPlayer {
    SLObjectItf slEngineObj_;
    SLEngineItf slEngineItf_;
    SLObjectItf outputMixObjectItf_;
    SLObjectItf playerObjectItf_;
    SLPlayItf playItf_;
    SLAndroidSimpleBufferQueueItf playBufferQueueItf_;
    FILE *inFile;
    SLuint32 frameSize;
    void *outBuf;
public:
    explicit AudioPlayer(const char *filename, uint32_t sampleRate, uint8_t channels, uint32_t bitPerChannel);
    void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);
    void getPcmData(void **pcm);
    ~AudioPlayer();
};


#endif //PULLPLAYER_AUDIO_PLAYER_H
