/**
 * description: 为了方便，设置全局参数。如：播放状态等 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/8
 */
#include "global.h"

Global::Global() {
    status = None;
    seekPos = 0;
    audioPktQ = new SafeQueue<AVPacket *>(DEFAULT_MAX_AUDIOQ_SIZE);
    videoPktQ = new SafeQueue<AVPacket *>(DEFAULT_MAX_VIDEOQ_SIZE);
//    videoFrameQ = new SafeQueue<AVFrame *>(DEFAULT_MAX_VIDEOQ_SIZE);
    audioPktQ->setReleaseCallback(releasePktCallback);
    videoPktQ->setReleaseCallback(releasePktCallback);
//    videoFrameQ->setReleaseCallback(releaseFrameCallback);
}

Global::~Global() {
    audioPktQ->clear();
    videoPktQ->clear();
//    videoFrameQ->clear();
    delete audioPktQ;
    delete videoPktQ;
//    delete videoFrameQ;
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
    env = nullptr;
}

void Global::setAllQueueWork(bool work) {
    audioPktQ->setWork(work);
    videoPktQ->setWork(work);
//    videoFrameQ->setWork(work);
}
