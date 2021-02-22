/**
 * description:   <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/1
 */

#include "hw_mediacodec.h"

static AVBufferRef *hw_device_ctx = NULL;
static enum AVPixelFormat hw_pix_fmt;
static FILE *output_file = NULL;

static int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type) {
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
                                      NULL, NULL, 0)) < 0) {
        HWLOGE("Failed to create specified HW device.\n");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    return err;
}

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                        const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    HWLOGE("Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

static int decode_write(AVCodecContext *avctx, AVPacket *packet) {
    AVFrame *frame = NULL, *sw_frame = NULL;
    AVFrame *tmp_frame = NULL;
    uint8_t *buffer = NULL;
    int size;
    int ret = 0;

    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0) {
        HWLOGE("avcodec_send_packet: %d(%s)", ret, av_err2str(ret));
        if(ret != AVERROR(EAGAIN)){
            return ret;
        }
    }

    while (1) {
        if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
            HWLOGE("Can not alloc frame\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        ret = avcodec_receive_frame(avctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            return 0;
        } else if (ret < 0) {
            HWLOGE("Error while decoding\n");
            goto fail;
        }

        HWLOGE("receive video frame success: pts=%lld",frame->pts);

        if (frame->format == hw_pix_fmt) {
            if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
                HWLOGE("Error transferring the data to system memory\n");
                goto fail;
            }
            tmp_frame = sw_frame;
        } else
            tmp_frame = frame;

        size = av_image_get_buffer_size(static_cast<AVPixelFormat>(tmp_frame->format),
                                        tmp_frame->width,
                                        tmp_frame->height, 1);
        buffer = static_cast<uint8_t *>(av_malloc(size));
        if (!buffer) {
            HWLOGE("Can not alloc buffer\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        //本次测试所得：AV_PIX_FMT_NV12
        ret = av_image_copy_to_buffer(buffer, size,
                                      (const uint8_t *const *) tmp_frame->data,
                                      (const int *) tmp_frame->linesize,
                                      static_cast<AVPixelFormat>(tmp_frame->format),
                                      tmp_frame->width, tmp_frame->height, 1);
        if (ret < 0) {
            HWLOGE("Can not copy image to buffer\n");
            goto fail;
        }

        if ((ret = fwrite(buffer, 1, size, output_file)) < 0) {
            HWLOGE("Failed to dump raw data.\n");
            goto fail;
        }

        fail:
        av_frame_free(&frame);
        av_frame_free(&sw_frame);
        av_freep(&buffer);
        if (ret < 0)
            return ret;
    }
}


void libffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vl) {
//    HWLOGD("%8s level=%3d fmt=%5s \n",ptr,level,fmt);
}


HWMediaCodec::HWMediaCodec(const char *srcFilename, const char *dstFilename) {
    AVFormatContext *input_ctx = NULL;
    int video_stream, ret;
//    AVStream *video = NULL;
    AVCodecContext *decoder_ctx = NULL;
    AVCodec *decoder = NULL;
    AVPacket packet;
    enum AVHWDeviceType type;
    int i;

    //设置输出，以及指定日志级别
    av_log_set_level(AV_LOG_DEBUG);
    av_log_set_flags(AV_LOG_SKIP_REPEATED);
    av_log_set_callback(libffmpeg_log_callback);

    //指定编码器类型
    type = av_hwdevice_find_type_by_name("mediacodec");
    if (type == AV_HWDEVICE_TYPE_NONE) {
        HWLOGE("Device type %s is not supported.\n", "mediacodec");
        HWLOGE("Available device types:");
        while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
            HWLOGE(" %s", av_hwdevice_get_type_name(type));
        HWLOGE("\n");
        return;
    }

    //解封装打开文件
    if ((ret = avformat_open_input(&input_ctx, srcFilename, NULL, NULL)) != 0) {
        HWLOGE("Couldn't open file %s: %d(%s)", srcFilename, ret, av_err2str(ret));
        return;
    }

    //填充流信息
    if (avformat_find_stream_info(input_ctx, NULL) < 0) {
        HWLOGE("Cannot find input stream information.\n");
        return;
    }

    //查找视频通道
    if ((video_stream = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0)) < 0) {
        HWLOGE("Cannot find a video stream in the input file\n");
        return;
    }

    if (!(decoder = avcodec_find_decoder_by_name("h264_mediacodec"))) {
        HWLOGE("avcodec_find_decoder_by_name failed.\n");
        return;
    }

    for (i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i);
        if (!config) {
            HWLOGE("Decoder %s does not support device type %s.\n",
                   decoder->name, av_hwdevice_get_type_name(type));
            return;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            config->device_type == type) {
            hw_pix_fmt = config->pix_fmt;
            break;
        }
    }

    if (!(decoder_ctx = avcodec_alloc_context3(decoder))) {
        HWLOGE("Cannot find a video stream in the input file%d", AVERROR(ENOMEM));
        return;
    }

    //将参数赋值给新的编码器
    if ((ret = avcodec_parameters_to_context(decoder_ctx, input_ctx->streams[video_stream]->codecpar)) < 0){
        HWLOGE("Couldn't copy param %d(%s)", ret, av_err2str(ret));
        return;
    }

    decoder_ctx->get_format = get_hw_format;

    if (hw_decoder_init(decoder_ctx, type) < 0) {
        return;
    }

    if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0) {
        HWLOGE("avcodec_open2: %d(%s)", ret, av_err2str(ret));
        return;
    }

    remove(dstFilename);
    output_file = fopen(dstFilename, "wb+");

    while (ret >= 0) {
        if ((ret = av_read_frame(input_ctx, &packet)) < 0)
            break;

        if (video_stream == packet.stream_index)
            ret = decode_write(decoder_ctx, &packet);

        av_packet_unref(&packet);
    }

    packet.data = NULL;
    packet.size = 0;
    ret = decode_write(decoder_ctx, &packet);
    av_packet_unref(&packet);

    if (output_file)
        fclose(output_file);
    avcodec_free_context(&decoder_ctx);
    avformat_close_input(&input_ctx);
    av_buffer_unref(&hw_device_ctx);
}
