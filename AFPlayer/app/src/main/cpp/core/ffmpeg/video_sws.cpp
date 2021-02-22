/**
 * description: 视频画面格式转换（其他格式转RGB24） <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/18
 */


#include "video_sws.h"

VideoSws::VideoSws(AVCodecContext *ctx, Global *g) {
    this->videoDecCtx = ctx;
    this->global = g;
}

VideoSws::~VideoSws() {
    videoDecCtx = nullptr;
    global = nullptr;
    if (swsCtx) {
        sws_freeContext(swsCtx);
        swsCtx = nullptr;
    }
}

int VideoSws::initSws() {
    int ret = -1;
    if (!(swsCtx = sws_getContext(videoDecCtx->width, videoDecCtx->height, videoDecCtx->pix_fmt,
                                  videoDecCtx->width, videoDecCtx->height, AV_PIX_FMT_RGB24,
                                  SWS_BILINEAR, nullptr, nullptr, nullptr))) {
        LOGE("Fail to create scale context!");
        return ret;
    }
    return 0;
}


void VideoSws::scaleVideo(AVFrame **frame) {
    AVFrame *re_frame;
    int ret;

    if (!(re_frame = av_frame_alloc())) {
        LOGE("Could not allocate sws frame!");
        return;
    }

    if ((ret = av_frame_copy_props(re_frame, *frame)) < 0) {
        LOGE("Copy video frame failed: %d(%s)", ret, av_err2str(ret));
        return;
    }

    re_frame->format = AV_PIX_FMT_RGB24;

    if ((ret = av_image_alloc(re_frame->data, re_frame->linesize,
                              videoDecCtx->width, videoDecCtx->height, AV_PIX_FMT_RGB24, 1)) <
        0) {
        LOGE("Could not allocate source image2: %d(%s)", ret, av_err2str(ret));
        return;
    }

    if ((ret = sws_scale(swsCtx, (const uint8_t *const *) (*frame)->data,
                         (*frame)->linesize, 0, videoDecCtx->height, re_frame->data,
                         re_frame->linesize)) < 0) {
        LOGE("Fail to scale video: %d(%s)", ret, av_err2str(ret));
        av_frame_free(&re_frame);
        return;
    }
    av_frame_free(frame);
    *frame = re_frame;
//    LOGD("scale video success!");
}