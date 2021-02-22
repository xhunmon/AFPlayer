/**
 * description: 使用OpenSL ES进行声音播放 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/9
 */
#include "sl_player.h"

static void *buffer;
static uint8_t *outBuffer;

SLPlayer::~SLPlayer() {
    if (playerObj) {
        (*playerObj)->Destroy(playerObj);
        playerObj = nullptr;
        playItf = nullptr;
        volumeItf = nullptr;
        bufferQueue = nullptr;
        buffer = nullptr;
        free(outBuffer);
        outBuffer = nullptr;
    }
    if (outputMixObj) {
        (*outputMixObj)->Destroy(outputMixObj);
        outputMixObj = nullptr;
    }

    if (engineObj) {
        (*engineObj)->Destroy(engineObj);
        engineObj = nullptr;
        engineItf = nullptr;
    }
}

int32_t SLPlayer::getFrameData(void **buf, uint8_t *outBuf) const {
    outBuf = new uint8_t[global->frameSize];
    AVFrame *frame;
    if (global->audioFrameQ->pop(frame)) {
        memset(outBuf, 0, global->frameSize);
        int index = 0;
        for (int i = 0; i < frame->nb_samples; i++)
            for (int ch = 0; ch < global->channels; ch++)
                memcpy(outBuf + (index++) * global->bytesPerSample,
                       frame->data[ch] + global->bytesPerSample * i, global->bytesPerSample);
//        memcpy(outBuf,frame->data,global->frameSize);
        *buf = outBuf;
        global->audioClock = frame->pts * av_q2d(global->audioTimeBase);
        LOGD("loading audio clock: %5lf",global->audioClock);
        av_frame_free(&frame);
        return global->frameSize;
    }
    return 0;
}


void playerCallback(SLAndroidSimpleBufferQueueItf bf, void *cxt) {
    auto *slPlayer = static_cast<SLPlayer *>(cxt);
    int32_t size = slPlayer->getFrameData(&buffer, outBuffer);
//    LOGD("playerCallback, size=%d", size);
    if (buffer && size > 0) {
        SLresult result = (*bf)->Enqueue(bf, buffer, size);
        if (checkError(&result, "Enqueue fail")) return;
//        LOGD("Enqueue success!");
    }
}

int SLPlayer::prepare(Global *g) {
    this->global = g;
    int ret = -1;
    SLresult result;

    if ((result = createEngine()) != SL_RESULT_SUCCESS)
        return ret;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
//    SLDataFormat_PCM format_pcm = {
//            SL_DATAFORMAT_PCM,
//            global->channels,
//            global->sampleRate * 1000,
//            getBitsPerSample(global->bytesPerSample),
//            getBitsPerSample(global->bytesPerSample),
//            getChannelMask(global->channels),
//            SL_BYTEORDER_LITTLEENDIAN
//    };

    SLAndroidDataFormat_PCM_EX pcmEx = {
            SL_ANDROID_DATAFORMAT_PCM_EX,
            global->channels,
            global->sampleRate * 1000,
            static_cast<SLuint32>(global->bytesPerSample*8),
            static_cast<SLuint32>(global->bytesPerSample*8),
            getChannelMask(global->channels),
            SL_BYTEORDER_LITTLEENDIAN,
            global->fmtType
    };

    SLDataSource audioSrc = {&loc_bufq, &pcmEx};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObj};
    SLDataSink audioSnk = {&loc_outmix, nullptr};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*engineItf)->CreateAudioPlayer(engineItf, &playerObj, &audioSrc, &audioSnk, 3, ids,
                                             req);
    if (checkError(&result, "create audio player obj fail")) return result;

    // realize the player
    result = (*playerObj)->Realize(playerObj, SL_BOOLEAN_FALSE);
    if (checkError(&result, "Realize audio player obj fail")) return result;

    // get the play interface
    result = (*playerObj)->GetInterface(playerObj, SL_IID_PLAY, &playItf);
    if (checkError(&result, "get audio player itf fail")) return result;

    // get the buffer queue interface
    result = (*playerObj)->GetInterface(playerObj, SL_IID_BUFFERQUEUE, &bufferQueue);
    if (checkError(&result, "get buffer queue itf fail")) return result;

    // register callback on the buffer queue
    result = (*bufferQueue)->RegisterCallback(bufferQueue, playerCallback, this);
    if (checkError(&result, "register player callback fail")) return result;

    // get the volume interface
    result = (*playerObj)->GetInterface(playerObj, SL_IID_VOLUME, &volumeItf);
    if (checkError(&result, "get volume itf fail")) return result;
    return 0;
}

int SLPlayer::start() {
    // set the player's state to playing
    SLresult result = (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    if (checkError(&result, "start playing fail")) return result;
//    主动调用回调函数开始工作
    playerCallback(bufferQueue, this);

    //静音：100*-50；最高：0
    result = (*volumeItf)->SetVolumeLevel(volumeItf, 50 * -50);
    if (checkError(&result, "set volume fail")) return result;
    return 0;
}

SLresult SLPlayer::createEngine() {
    LOGD("start create engine");
    SLresult result;
    // create engine
    result = slCreateEngine(&engineObj, 0, nullptr, 0, nullptr, nullptr);
    if (checkError(&result, "create engine obj fail")) return result;

    // realize the engine
    result = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    if (checkError(&result, "Realize engine obj fail")) return result;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineItf);
    if (checkError(&result, "Realize engine itf fail")) return result;

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineItf)->CreateOutputMix(engineItf, &outputMixObj, 1, ids, req);
    if (checkError(&result, "create outputMix fail")) return result;

    // realize the output mix
    result = (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE);
    if (checkError(&result, "Realize outputMix fail")) return result;

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObj)->GetInterface(outputMixObj, SL_IID_ENVIRONMENTALREVERB,
                                           &outputMixEnvironmentalReverb);
    if (checkError(&result, "get environmental reverb fail")) return result;
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        if (checkError(&result, "set environmental reverb fail")) return result;
    }
    LOGD("success create engine");
    return result;
}