/**
 * description: 使用ffmpeg进行解码 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/7
 */
#ifndef AFPLAYER_FF_DECODER_H
#define AFPLAYER_FF_DECODER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
}

#include "../common/android_log.h"
#include "../common/macro.h"
#include "../common/global.h"
#include <cstdlib>
#include <unistd.h>
#include "audio_swr.h"
#include "video_sws.h"

class FFDecoder {
public:
    AudioSwr *audioSwr = nullptr;
    VideoSws *videoSws = nullptr;
    AVFormatContext *iFmtCtx = nullptr;
    AVCodecContext *videoDecCtx = nullptr;
    AVCodecContext *audioDecCtx = nullptr;
    int videoIndex = -1;
    int audioIndex = -1;
    AVHWDeviceType hwType = AV_HWDEVICE_TYPE_NONE;
    AVCodec *videoDecoder = nullptr;
    Global *global = nullptr;
    SLuint32 outFmtType = SL_ANDROID_PCM_REPRESENTATION_FLOAT;//OpenSL ES播放值的类型，通用

    void decodeThread();

    FFDecoder(Global *global);

    int setDataSource(const char *src, char **errMsg);

    ~FFDecoder();

    void demuxer() const;

    void decodeAudio() const;

    void decodeVideo() const;

private:
    pthread_t pktPid;
    pthread_t audioFramePid;
    pthread_t videoFramePid;

    bool isSupportHardware();

    int createDecoderContext(int index, bool isSupportHardware, char **errMsg);

    static void logCallback(void *ptr, int level, const char *fmt, va_list vl);

    int initAudioSwr();

    int initVideoSws();

    int decodeAndPushVideo(AVCodecContext *avctx, AVPacket *packet) const;
};


#endif //AFPLAYER_FF_DECODER_H
