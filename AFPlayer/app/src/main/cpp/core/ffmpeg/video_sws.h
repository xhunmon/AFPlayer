/**
 * description: 视频画面格式转换（其他格式转RGB24） <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/18
 */

#ifndef AFPLAYER_VIDEO_SWS_H
#define AFPLAYER_VIDEO_SWS_H

#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}
#include "../common/global.h"

class VideoSws {
private:
    SwsContext *swsCtx = nullptr;
    AVCodecContext *videoDecCtx = nullptr;
    Global *global = nullptr;

public:
    VideoSws(AVCodecContext *videoDecCtx, Global *global);
    ~VideoSws();

    int initSws();

    void scaleVideo(AVFrame **frame);
};


#endif //AFPLAYER_VIDEO_SWS_H
