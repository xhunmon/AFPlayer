/**
 * description: 为了方便，设置全局参数。如：播放状态等 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/8
 */
#include "global.h"

static void releasePktCallback(AVPacket **pkt) {
    if (pkt) {
        LOGD("release packet pts: %5lld \tpos: %5lld ", (*pkt)->pts, (*pkt)->pos);
        av_packet_free(pkt);
        *pkt = nullptr;
    }
}

static void releaseFrameCallback(AVFrame **frame) {
    if (frame) {
        LOGD("release packet frame: %5lld \tpos: %5lld ", (*frame)->pts, (*frame)->pkt_pos);
        av_frame_free(frame);
        *frame = nullptr;
    }
}

Global::Global() {
    status = None;
    seekPos = 0;
    audioPktQ = new SafeQueue<AVPacket *>(DEFAULT_MAX_AUDIOQ_SIZE);
    videoPktQ = new SafeQueue<AVPacket *>(DEFAULT_MAX_VIDEOQ_SIZE);
    audioFrameQ = new SafeQueue<AVFrame *>(DEFAULT_MAX_AUDIOQ_SIZE);
    videoFrameQ = new SafeQueue<AVFrame *>(DEFAULT_MAX_VIDEOQ_SIZE);
    audioPktQ->setReleaseCallback(releasePktCallback);
    videoPktQ->setReleaseCallback(releasePktCallback);
    audioFrameQ->setReleaseCallback(releaseFrameCallback);
    videoFrameQ->setReleaseCallback(releaseFrameCallback);
}

Global::~Global() {
    audioPktQ->clear();
    videoPktQ->clear();
    audioFrameQ->clear();
    videoFrameQ->clear();
    delete audioPktQ;
    delete videoPktQ;
    delete audioFrameQ;
    delete videoFrameQ;
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
    env = nullptr;
}

void Global::setAllQueueWork(bool work) {
    audioPktQ->setWork(work);
    videoPktQ->setWork(work);
    audioFrameQ->setWork(work);
    videoFrameQ->setWork(work);
}
