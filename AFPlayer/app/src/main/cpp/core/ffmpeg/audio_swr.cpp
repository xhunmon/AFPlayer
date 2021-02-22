/**
 * description: 使用ffmpeg对音频数据进行重采样 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/10
 */



#include "audio_swr.h"

AudioSwr::AudioSwr(AVCodecContext *d, Global *g) {
    decCtx = d;
    global = g;
}

bool AudioSwr::isNeedSwr() {
    bool needSampleRate;
    bool needSampleFmt;
    switch (decCtx->sample_rate * 1000) {//需要根据OpenSL ES所支持的来判断是否需要重采样。
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
            outSampleRate = decCtx->sample_rate;
            break;
        default:
            LOGE("OpenSL ES not support the %d sample_rate.", decCtx->sample_rate);
            needSampleRate = true;
            outSampleRate = DEFAULT_AUDIO_SAMPLE_RATE;
            break;
    }


    //OpenSL ES 1.1支持浮点的值
    switch (decCtx->sample_fmt) {
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            needSampleFmt = false;
            outSampleFmt = decCtx->sample_fmt;
            outFmtType = SL_ANDROID_PCM_REPRESENTATION_FLOAT;
            break;
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S16P:
        case AV_SAMPLE_FMT_S32P:
        case AV_SAMPLE_FMT_S64:
        case AV_SAMPLE_FMT_S64P:
            needSampleFmt = false;
            outSampleFmt = decCtx->sample_fmt;
            outFmtType = SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT;
            break;
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_U8P:
            needSampleFmt = false;
            outSampleFmt = decCtx->sample_fmt;
            outFmtType = SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT;
            break;
        default:
            LOGE("OpenSL ES not support the %d sample_fmt.", decCtx->sample_fmt);
            needSampleFmt = true;
            outSampleFmt = DEFAULT_AUDIO_FORMAT;
            outFmtType = SL_ANDROID_PCM_REPRESENTATION_FLOAT;
            break;
    }

    return needSampleRate || needSampleFmt;
}

bool AudioSwr::initSwr() {
    bool ret = false;
    if (!(swrCtx = swr_alloc())) {
        LOGE("Could not allocate swr: %d(%s)", ret, av_err2str(ret));
        return ret;
    }

    av_opt_set_int(swrCtx, "in_channel_count", decCtx->channels, 0);
    av_opt_set_int(swrCtx, "in_sample_rate", decCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", decCtx->sample_fmt, 0);
    av_opt_set_int(swrCtx, "out_channel_count", decCtx->channels, 0);
    av_opt_set_int(swrCtx, "out_sample_rate", outSampleRate, 0);
    av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", (AVSampleFormat) outSampleFmt, 0);

    if (swr_init(swrCtx) < 0) {
        LOGE("Failed to initialize swr: %d(%s)", ret, av_err2str(ret));
        return ret;
    }
    return true;
}

AudioSwr::~AudioSwr() {
    decCtx = nullptr;
    global = nullptr;
    if (swrCtx) {
        swr_free(&swrCtx);
        swrCtx = nullptr;
    }

    outSampleRate = 0;
    outSampleFmt = AV_SAMPLE_FMT_NONE;
    nbSample = 0;
    outFmtType = 0;
}

void AudioSwr::resampleAudio(AVFrame **frame) {
    AVFrame *re_frame;
    int ret;
    if (!(re_frame = av_frame_alloc())) {
        LOGE("Could not allocate swr frame!");
        return;
    }

    if ((ret = av_frame_copy_props(re_frame, *frame)) < 0) {
        LOGE("Copy audio frame failed: %d(%s)", ret, av_err2str(ret));
        return;
    }

    //需要改变的
    re_frame->format = outSampleFmt;
    re_frame->channel_layout = (*frame)->channel_layout;
    re_frame->channels = (*frame)->channels;
    re_frame->sample_rate = outSampleRate;
    re_frame->nb_samples = nbSample;

    if ((ret = av_frame_get_buffer(re_frame, 0)) < 0) {
        LOGE("Fail to get buffer: %d(%s)", ret, av_err2str(ret));
        return;
    }

    int64_t delays = swr_get_delay(swrCtx,(*frame)->sample_rate);
    int64_t max_samples =  av_rescale_rnd(delays + (*frame)->nb_samples,global->sampleRate,(*frame)->sample_rate,AV_ROUND_UP);
//    int samples = swr_convert(swrCtx, re_frame->data, max_samples, (const uint8_t **) (*frame)->data, (*frame)->nb_samples);
    if ((ret = swr_convert(swrCtx, re_frame->data, max_samples, (const uint8_t **) (*frame)->data, (*frame)->nb_samples)) < 0) {
        LOGE("Fail to resample: %d(%s)", ret, av_err2str(ret));
        av_frame_free(&re_frame);
        return;
    }
    //获得   samples 个   * 2 声道 * 2字节（16位）
//    global->frameSize =  samples * global->channels * global->bytesPerSample;
    //获取 frame 的一个相对播放时间 （相对开始播放）
    // 获得 相对播放这一段数据的秒数
    re_frame->pts = (*frame)->pts * av_q2d(decCtx->time_base);//time_base应该是stream的

    /*if ((ret = swr_convert(swrCtx, re_frame->data, nbSample,
                           (const uint8_t **) (*frame)->data,
                           (*frame)->nb_samples)) < 0) {
        LOGE("Fail to resample: %d(%s)", ret, av_err2str(ret));
        av_frame_free(&re_frame);
        return;
    }*/
    av_frame_free(frame);
    *frame = re_frame;
//    LOGD("resample audio success!");
}
