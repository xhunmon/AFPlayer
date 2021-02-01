/**
 * description:   <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/1
 */


#ifndef PULLPLAYER_HW_MEDIACODEC_H
#define PULLPLAYER_HW_MEDIACODEC_H

#include <android/log.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
};

#define AV_LOG_INFO

#define HW_DEBUG_TAG "HWMediaCodec:Native"
#define HWLOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, HW_DEBUG_TAG, __VA_ARGS__))
#define HWLOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, HW_DEBUG_TAG, __VA_ARGS__))


class HWMediaCodec {
public:
    HWMediaCodec(const char *srcFilename, const char *dstFilename);
};


#endif //PULLPLAYER_HW_MEDIACODEC_H
