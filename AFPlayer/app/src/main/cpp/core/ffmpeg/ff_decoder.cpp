/**
 * description: 使用ffmpeg进行解码 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/7
 */

#include "ff_decoder.h"

static AVPixelFormat hwPixFmt = AV_PIX_FMT_NONE;
static AVBufferRef *hw_device_ctx = nullptr;

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;
    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hwPixFmt) {
            LOGD("get hw format success.\n");
            return *p;
        }
    }
    LOGE("Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

static int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type) {
    int err = 0;
    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
                                      nullptr, nullptr, 0)) < 0) {
        LOGE("Failed to create specified HW device.\n");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    return err;
}


FFDecoder::FFDecoder(Global *global) {
    this->global = global;
    //设置输出，以及指定日志级别
    av_log_set_level(AV_LOG_DEBUG);
    av_log_set_flags(AV_LOG_SKIP_REPEATED);
    av_log_set_callback(logCallback);
}

FFDecoder::~FFDecoder() {
    if (videoDecCtx) {
        avcodec_free_context(&videoDecCtx);
    }
    if (audioDecCtx) {
        avcodec_free_context(&audioDecCtx);
    }
    if (iFmtCtx) {
        avformat_close_input(&iFmtCtx);
    }
    if (videoSws) {
        delete videoSws;
        videoSws = nullptr;
    }
    videoDecoder = nullptr;
    videoDecCtx = nullptr;
    audioDecCtx = nullptr;
    iFmtCtx = nullptr;
    global = nullptr;
    hwPixFmt = AV_PIX_FMT_NONE;
    LOGD("release FFDecoder finnish!");
}

int FFDecoder::prepare(const char *src, char **errMsg) {
    int ret;
    if ((ret = avformat_open_input(&iFmtCtx, src, nullptr, nullptr)) != 0) {
        LOGE("Couldn't open file %s: %d(%s)", src, ret, av_err2str(ret));
        *errMsg = new char[]{"打开文件失败"};
        return ERR_OPEN_FAIL;
    }

    if ((ret = avformat_find_stream_info(iFmtCtx, nullptr)) < 0) {
        LOGE("Cannot find input stream information %s: %d(%s)", src, ret, av_err2str(ret));
        *errMsg = new char[]{"没有找到流"};
        return ERR_NO_STREAM;
    }

    if ((videoIndex = av_find_best_stream(iFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0)) < 0) {
        LOGE("Cannot find video stream %s: %d(%s)", src, ret, av_err2str(ret));
    }

    if ((audioIndex = av_find_best_stream(iFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0)) < 0) {
        LOGE("Cannot find audio stream %s: %d(%s)", src, ret, av_err2str(ret));
    }

    if (videoIndex == -1 && audioIndex == -1) {
        *errMsg = new char[]{"无音频视频流"};
        return ERR_NO_VIDEO_AND_AUDIO_STREAM;
    }

    if (isSupportHardware()) {
        LOGD("use hardware decode!");
        if ((ret = createDecoderContext(videoIndex, true, errMsg)) < 0) {
            return ret;
        }
    } else {
        if (videoIndex >= 0 &&
            (ret = createDecoderContext(videoIndex, false, errMsg)) < 0) {
            return ret;
        }
    }
    if (audioIndex >= 0 &&
        (ret = createDecoderContext(audioIndex, false, errMsg)) < 0) {
        return ret;
    }

    if (audioIndex >= 0) {
        if ((ret = initAudioSwr()) < 0) return ret;
    }

    if (videoIndex >= 0) {
        if ((ret = initVideoSws()) < 0) return ret;
    }

    return ret;
}

void FFDecoder::logCallback(void *ptr, int level, const char *fmt, va_list vl) {
//    LOGD("%8s level=%3d fmt=%5s \n",ptr,level,fmt);
}

bool FFDecoder::isSupportHardware() {
    if (!global->openHardwareDecode) {
        return false;
    }
    if (videoIndex < 0) {//没有视频流，不需要硬解码
        return false;
    }
    //指定编码器类型
    hwType = av_hwdevice_find_type_by_name("mediacodec");
    if (hwType == AV_HWDEVICE_TYPE_NONE) {
        LOGE("Cannot support mediacodec!");
        return false;
    }
    AVCodecID videoId = iFmtCtx->streams[videoIndex]->codecpar->codec_id;
    switch (videoId) {
        case AV_CODEC_ID_H264: {
            if (!(videoDecoder = avcodec_find_decoder_by_name("h264_mediacodec"))) {
                LOGE("Cannot support mediacodec, because h264_mediacodec not found!");
                return false;
            }
            break;
        }
        case AV_CODEC_ID_HEVC: {
            if (!(videoDecoder = avcodec_find_decoder_by_name("hevc_mediacodec"))) {
                LOGE("Cannot support mediacodec, because hevc_mediacodec not found!");
                return false;
            }
            break;
        }
        case AV_CODEC_ID_MPEG4: {
            if (!(videoDecoder = avcodec_find_decoder_by_name("mpeg4_mediacodec"))) {
                LOGE("Cannot support mediacodec, because mpeg4_mediacodec not found!");
                return false;
            }
            break;
        }
        case AV_CODEC_ID_VP8: {
            if (!(videoDecoder = avcodec_find_decoder_by_name("vp8_mediacodec"))) {
                LOGE("Cannot support mediacodec, because vp8_mediacodec not found!");
                return false;
            }
            break;
        }
        case AV_CODEC_ID_VP9: {
            if (!(videoDecoder = avcodec_find_decoder_by_name("vp9_mediacodec"))) {
                LOGE("Cannot support mediacodec, because vp9_mediacodec not found!");
                return false;
            }
            break;
        }
        default:
            LOGE("Cannot support mediacodec, because found nothing!");
            return false;
    }

    for (int i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(videoDecoder, i);
        if (!config) {
            LOGE("Decoder %s does not support device type %s.\n",
                 videoDecoder->name, av_hwdevice_get_type_name(hwType));
            return false;
        }
        unsigned int methods = config->methods;//methods的值总为正，所以直接转unsigned来消除警告
        if (methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == hwType) {
            hwPixFmt = config->pix_fmt;
            break;
        }
    }
    return true;
}

int FFDecoder::createDecoderContext(int index, bool isSupportHardware, char **errMsg) {
    AVStream *stream = iFmtCtx->streams[index];
    AVCodec *codec;
    AVCodecContext *decCtx;
    int ret;
    if (isSupportHardware) {
        codec = videoDecoder;
    } else {
        if (!(codec = avcodec_find_decoder(iFmtCtx->streams[index]->codecpar->codec_id))) {
            LOGE("Cannot found decoder!");
            *errMsg = new char[]{"无解码器"};
            return ERR_NO_DECODER;
        }
    }

    if (!(decCtx = avcodec_alloc_context3(codec))) {
        LOGE("Failed to allocate the decoder context for stream #%u\n", index);
        *errMsg = new char[]{"申请内存失败"};
        return ERR_ALLOCATE_FAIL;
    }

    if ((ret = avcodec_parameters_to_context(decCtx, stream->codecpar)) < 0) {
        LOGE("Failed to copy decoder parameters to input decoder context for stream: %d(%s)", ret,
             av_err2str(ret));
        *errMsg = new char[]{"复制解码器参数失败"};
        return ERR_COPY_DECODER_FAIL;
    }

    if (isSupportHardware) {
        decCtx->get_format = get_hw_format;
        if (hw_decoder_init(decCtx, hwType) < 0) {
            LOGE("Failed to create specified HW device: %d(%s)", ret, av_err2str(ret));
            *errMsg = new char[]{"创建硬解码器失败"};
            return ERR_CREATE_HW_DEVICE;
        }
    }

    if (decCtx->codec_type == AVMEDIA_TYPE_VIDEO) {
        decCtx->framerate = av_guess_frame_rate(iFmtCtx, stream, nullptr);
    }

    if ((ret = avcodec_open2(decCtx, codec, nullptr)) < 0) {
        LOGE("Failed to open decoder for stream: %d(%s)", ret, av_err2str(ret));
        *errMsg = new char[]{"打开解码器失败"};
        return ERR_DECODER_OPEN_FIAL;
    }

    if (index == videoIndex) {
        videoDecCtx = decCtx;
    } else if (index == audioIndex) {
        audioDecCtx = decCtx;
    }
    return 0;
}

static void *demuxerThread(void *data) {
    auto *ffDecoder = (FFDecoder *) data;
    ffDecoder->demuxer();
    pthread_exit(&ffDecoder->pktPid);
}

void FFDecoder::decodeThread() {
    pthread_create(&pktPid, nullptr, demuxerThread, this);
}

void FFDecoder::demuxer() const {
    int ret;
    AVPacket *pkt;
    while (global->status == Playing) {
        if (global->seekPos > 0) {//从指定位置开始解码
//            ret = avformat_seek_file(iFmtCtx, -1, seek_min, seek_target, seek_max, 1);
        }
        if (!(pkt = av_packet_alloc())) {
            LOGE("Error alloc packet!");
            releasePktCallback(&pkt);
            break;
        }
        if ((ret = av_read_frame(iFmtCtx, pkt)) < 0) {//已解码完成
            LOGE("read frame fail: %d(%s)", ret, av_err2str(ret));
            releasePktCallback(&pkt);
            break;
        }
        if (pkt->stream_index == audioIndex) {
            global->audioPktQ->push(pkt);
        } else if (pkt->stream_index == videoIndex) {
            global->videoPktQ->push(pkt);
//            releasePktCallback(&pkt);
        }
    }
}

void FFDecoder::decodeVideo() const {
    AVPacket *pkt;
    while (global->status == Playing || global->status == Paused) {
        while (global->status == Paused) {
            usleep(10);
        }
        if (pkt) {
            av_packet_unref(pkt);
        }
        if (!global->videoPktQ->pop(pkt)) {
            LOGD("decode video packet finish!");
            break;
        }

        if (decodeAndPushVideo(videoDecCtx, pkt) < 0) {
            av_packet_unref(pkt);
            pkt->data = nullptr;
            pkt->size = 0;
            decodeAndPushVideo(videoDecCtx, pkt);
            av_packet_unref(pkt);
            break;
        }
        av_packet_unref(pkt);
    }
    LOGD("decode video finish! ");
}

int FFDecoder::initAudioSwr() {
    int ret = 1;
    return ret;
}

int FFDecoder::initVideoSws() {
//    videoSws = new VideoSws(videoDecCtx, global);
//    return videoSws->initSws();
    return 1;
}

int FFDecoder::decodeAndPushVideo(AVCodecContext *avctx, AVPacket *packet) const {
    AVFrame *frame = nullptr, *sw_frame = nullptr;
    AVFrame *tmp_frame = nullptr;
    int ret;
    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0) {
        LOGE("Fail send video packet: %d(%s)", ret, av_err2str(ret));
        if (ret != AVERROR(EAGAIN)) {
            return ret;
        }
    }
    while (true) {
        if (!(frame = av_frame_alloc())) {
            LOGE("Error alloc video frame\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        ret = avcodec_receive_frame(avctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            return 0;
        } else if (ret < 0) {
            LOGE("Error while decoding\n");
            goto fail;
        }

        LOGD("receive video frame success: pts=%5lld, videoClock=%5lf", frame->pts,
             frame->pts * av_q2d(global->videoTimeBase));

        if (frame->format == hwPixFmt) {
            if (!(sw_frame = av_frame_alloc())) {
                LOGE("Error alloc video sw frame\n");
                ret = AVERROR(ENOMEM);
                av_frame_free(&frame);
                goto fail;
            }
            if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
                LOGE("Error transferring the data to system memory\n");
                av_frame_free(&frame);
                av_frame_free(&sw_frame);
                goto fail;
            }
            tmp_frame = sw_frame;
            av_frame_free(&frame);
        } else {
            tmp_frame = frame;
        }

        //将格式转换成RGB24，以便OpenGL ES播放
        if (videoSws) {
            videoSws->scaleVideo(&tmp_frame);
        }

//        global->videoFrameQ->push(tmp_frame);
    }

    fail:
    return ret;
}


