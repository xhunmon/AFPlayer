/**
 * description:   <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/1/26
 */

#include <cstdlib>
#include <android/log.h>
#include "audio_player.h"


void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *ctx) {
    (static_cast<AudioPlayer *>(ctx))->ProcessSLCallback(bq);
}


AudioPlayer::AudioPlayer(const char *filename, uint32_t sampleRate, uint8_t channels,
                         uint32_t bitPerChannel) {

    //一帧的大小
    frameSize = sampleRate * channels * bitPerChannel / 8;
    inFile = fopen(filename, "rb+");

    SLresult result;
    //1.1）创建引擎对象。引擎对象是OpenSL ES提供API的唯一入
    result = slCreateEngine(&slEngineObj_, 0, nullptr, 0, NULL, NULL);
    SLASSERT(result);//断言，用于调试，能快速定位问题
    //1.2）实例化引擎对象，需要通过在第1步得到的引擎对象接口来实例化(在ELSE中，任何对象都需要使用接口来进行实例化)
    result = (*slEngineObj_)->Realize(slEngineObj_, SL_BOOLEAN_FALSE);
    SLASSERT(result);
    //1.3）获取这个引擎对象的方法接口，通过GetInterface方法，使用第2步已经实例化好了的对象
    result = (*slEngineObj_)->GetInterface(slEngineObj_, SL_IID_ENGINE, &slEngineItf_);
    SLASSERT(result);

    //2.创建混音器对象
    result = (*slEngineItf_)->CreateOutputMix(slEngineItf_, &outputMixObjectItf_, 0, NULL, NULL);
    SLASSERT(result);
    result = (*outputMixObjectItf_)->Realize(outputMixObjectItf_, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    //3、创建播放器
    // numBuffers：设置4个缓冲数据
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            channels,//声道数（立体声）
            sampleRate * 1000,//44100hz -> 44100000 的频率；参考：SL_SAMPLINGRATE_44_1
            bitPerChannel == 32 ?
            SL_PCMSAMPLEFORMAT_FIXED_32 : SL_PCMSAMPLEFORMAT_FIXED_16,//位数 32位
            bitPerChannel == 32 ?
            SL_PCMSAMPLEFORMAT_FIXED_32 : SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//小端排序
    };
    SLDataSource slDataSource = {&android_queue, &pcm};
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObjectItf_};
    SLDataSink audioSnk = {&outputMix, NULL};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*slEngineItf_)->CreateAudioPlayer(slEngineItf_, &playerObjectItf_, &slDataSource,
                                                &audioSnk, 3, ids, req);
    SLASSERT(result);

    result = (*playerObjectItf_)->Realize(playerObjectItf_, SL_BOOLEAN_FALSE);
    SLASSERT(result);
    result = (*playerObjectItf_)->GetInterface(playerObjectItf_, SL_IID_PLAY, &playItf_);
    SLASSERT(result);

    //4、获取播放器对象的数据队列接口
    result = (*playerObjectItf_)
            ->GetInterface(playerObjectItf_, SL_IID_BUFFERQUEUE, &playBufferQueueItf_);
    SLASSERT(result);

    //5. 设置回调函数
    result = (*playBufferQueueItf_)
            ->RegisterCallback(playBufferQueueItf_, bqPlayerCallback, this);
    SLASSERT(result);

    //6. 获取播放状态接口
    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_PLAYING);
    SLASSERT(result);

    //7. 主动调用回调函数开始工作
    ProcessSLCallback(playBufferQueueItf_);
}

void AudioPlayer::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq) {
    void *buffer;
    getPcmData(&buffer);
    if (NULL != buffer) {
        SLresult result;
        result = (*bq)->Enqueue(bq, buffer, frameSize);
        SLASSERT(result);
    }
}

void AudioPlayer::getPcmData(void **pcm) {
    outBuf = new uint32_t[frameSize];
    while (!feof(inFile)) {
        //读取一帧数据
        memset(outBuf, 0, frameSize);
        fread(outBuf, frameSize, 1, inFile);
        if (outBuf == nullptr) {//读取结束
            break;
        }
        *pcm = outBuf;
        goto end;
    }
    __android_log_print(ANDROID_LOG_DEBUG, "audio_play", "red finish?");
    (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
    end:
    __android_log_print(ANDROID_LOG_DEBUG, "audio_play", "playing？");
}

AudioPlayer::~AudioPlayer() {
    SLuint32 state;
    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    if (result == SL_RESULT_SUCCESS) {
        if (state == SL_PLAYSTATE_PLAYING) {
            __android_log_print(ANDROID_LOG_DEBUG, "audio_play", "SL_PLAYSTATE_PLAYING");
        } else if (state == SL_PLAYSTATE_PAUSED) {
            __android_log_print(ANDROID_LOG_DEBUG, "audio_play", "SL_PLAYSTATE_PAUSED");
        } else if (state == SL_PLAYSTATE_STOPPED) {
            __android_log_print(ANDROID_LOG_DEBUG, "audio_play", "SL_PLAYSTATE_STOPPED");
        }
    }

    if (playerObjectItf_ != NULL) {
        (*playerObjectItf_)->Destroy(playerObjectItf_);
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObjectItf_) {
        (*outputMixObjectItf_)->Destroy(outputMixObjectItf_);
    }

    if (slEngineObj_) {
        (*slEngineObj_)->Destroy(slEngineObj_);
    }

    delete outBuf;
}
