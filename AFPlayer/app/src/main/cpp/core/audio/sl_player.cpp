/**
 * description: 使用OpenSL ES进行声音播放 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/9
 */
#include "sl_player.h"

SLPlayer::SLPlayer(AVCodecContext *audioDecCtx) {
    this->audioDecCtx = audioDecCtx;
}

SLPlayer::~SLPlayer() {
    if (playerObj) {
        (*playerObj)->Destroy(playerObj);
        playerObj = nullptr;
        playItf = nullptr;
        volumeItf = nullptr;
        bufferQueue = nullptr;
        buffer = nullptr;
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

void playerCallback(SLAndroidSimpleBufferQueueItf bf, void *cxt) {
    auto *slPlayer = static_cast<SLPlayer *>(cxt);
    int32_t size = slPlayer->decodeAudio();
    if (slPlayer->buffer && size > 0) {
        slPlayer->global->audioClock += size / ((double) (slPlayer->global->sampleRate * 2 * 2));
        if (slPlayer->global->audioClock - slPlayer->global->lastUpTime >= 0.2) {//回调给java层
            slPlayer->global->lastUpTime = slPlayer->global->audioClock;
            slPlayer->global->callJava->onPlaying(Thread_child, slPlayer->global->audioClock,
                                                  slPlayer->global->duration);
        }
        SLresult result = (*bf)->Enqueue(bf, slPlayer->buffer, size);
        if (checkError(&result, "Enqueue fail")) return;
    }
}

int SLPlayer::prepare(Global *g) {
    this->global = g;
    AVSampleFormat *outSampleFmt = nullptr;
    int outSampleRate = 0;
    isNeedSwr(audioDecCtx->sample_rate, audioDecCtx->sample_fmt, outSampleRate, &outSampleFmt);
    global->sampleFormat = *outSampleFmt;
    global->sampleRate = outSampleRate;
    global->bytesPerSample = av_get_bytes_per_sample(*outSampleFmt);
    global->nbSample = 1024;
    global->channels = audioDecCtx->channels;
    global->frameSize = av_samples_get_buffer_size(nullptr,
                                                   global->channels,
                                                   global->nbSample,
                                                   *outSampleFmt,
                                                   1);
    buffer = (uint8_t *) av_malloc(audioDecCtx->sample_rate * 2 * 2);
    initOpenSl();
    return 0;
}

void *playAudioThread(void *data) {
    SLPlayer *player = (SLPlayer *) data;
//    player->initOpenSl();
    player->start();
    pthread_exit(&player->pidPlay);
}

void SLPlayer::start() {
    // set the player's state to playing
    SLresult result = (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    if (checkError(&result, "play playing fail")) return;
//    主动调用回调函数开始工作
    playerCallback(bufferQueue, this);

    //静音：100*-50；最高：0
//    result = (*volumeItf)->SetVolumeLevel(volumeItf, 50 * -50);
//    if (checkError(&result, "set volume fail")) return;
}

void SLPlayer::play() {
    pthread_create(&pidPlay, nullptr, playAudioThread, this);
}

SLresult SLPlayer::createEngine() {
    LOGD("play create engine");
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

int SLPlayer::decodeAudio() {
    int ret;
    while (global->status == Playing) {
        if (!(pkt = av_packet_alloc())) {
            LOGE("Error alloc audio packet\n");
            releasePktCallback(&pkt);
            continue;
        }
        if (!global->audioPktQ->pop(pkt)) {
            releasePktCallback(&pkt);
            continue;
        }
        if (avcodec_send_packet(audioDecCtx, pkt) != 0) {
            releasePktCallback(&pkt);
            continue;
        }
        if (!(frame = av_frame_alloc())) {
            LOGE("Error alloc audio frame\n");
            releasePktCallback(&pkt);
            releaseFrameCallback(&frame);
            continue;
        }
        ret = avcodec_receive_frame(audioDecCtx, frame);
        if (ret != 0) {
            releasePktCallback(&pkt);
            releaseFrameCallback(&frame);
            continue;
        }

        if (frame->channels && frame->channel_layout == 0) {
            frame->channel_layout = av_get_default_channel_layout(frame->channels);
        } else if (frame->channels == 0 && frame->channel_layout > 0) {
            frame->channels = av_get_channel_layout_nb_channels(frame->channel_layout);
        }

        SwrContext *swr_ctx;

        swr_ctx = swr_alloc_set_opts(
                nullptr,
                AV_CH_LAYOUT_STEREO,
                AV_SAMPLE_FMT_S16,
                frame->sample_rate,
                frame->channel_layout,
                (AVSampleFormat) frame->format,
                frame->sample_rate,
                0, nullptr
        );

        if (!swr_ctx || swr_init(swr_ctx) < 0) {
            releasePktCallback(&pkt);
            releaseFrameCallback(&frame);
            swr_free(&swr_ctx);
            continue;
        }
        int nb = swr_convert(
                swr_ctx,
                &buffer,
                frame->nb_samples,
                (const uint8_t **) frame->data,
                frame->nb_samples);
        int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
        data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
        global->nowTime = frame->pts * av_q2d(global->audioTimeBase);
        if (global->nowTime < global->audioClock) {
            global->nowTime = global->audioClock;
        }
//        global->videoClock = global->nowTime;
        releasePktCallback(&pkt);
        releaseFrameCallback(&frame);
        swr_free(&swr_ctx);
        return data_size;

    }
    return 0;
}

int SLPlayer::initOpenSl() {
    int ret = -1;
    SLresult result;

    if ((result = createEngine()) != SL_RESULT_SUCCESS)
        return ret;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    SLDataFormat_PCM pcmEx = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            global->channels,//2个声道（立体声）
            static_cast<SLuint32>(getCurrentSampleRateForSl(global->sampleRate)),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            getChannelMask(global->channels),//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
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

    //静音：100*-50；最高：0
    result = (*volumeItf)->SetVolumeLevel(volumeItf, 50 * -20);
    if (checkError(&result, "set volume fail")) return result;
    return 0;
}
