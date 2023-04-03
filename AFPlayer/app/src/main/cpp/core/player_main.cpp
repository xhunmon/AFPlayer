/**
 * description: 整个底层播放器入口，对接 native_player_jni.cpp 中所有接口参数 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/7
 */
#include "player_main.h"

void Player::setJavaStatucCallback(JavaVM *javaVm, JNIEnv *env, jobject *onAFCallback) {
    if (callJava) {//已存在的情况下，需要把旧的给注销
        delete callJava;
        callJava = nullptr;
    }
    callJava = new CallJava(javaVm, env, *onAFCallback);
    global->callJava = callJava;
}

static void *preparedThread(void *data) {
    Player *player = (Player *) data;
    player->prepare();
    pthread_exit(&player->pidPrepare);
}


void Player::prepare() {
//通过ffmpeg进行解析，优先使用硬解码
    if (ffDecoder) {
        delete ffDecoder;
        ffDecoder = nullptr;
    }
    ffDecoder = new FFDecoder(global);
    int ret;
    char *errMsg = nullptr;
    if ((ret = ffDecoder->prepare(src, &errMsg)) < 0) {
        delete ffDecoder;
        ffDecoder = nullptr;
        if (callJava) {
            callJava->onError(Thread_child, ret, errMsg);
        }
        return;
    }
    slPlayer = new SLPlayer(ffDecoder->audioDecCtx);
    global->duration = ffDecoder->iFmtCtx->duration / AV_TIME_BASE;
    //根据解码信息进行初始化全局的播放相关参数
    this->initPlayerGlobal(window, env);

    //初始化播放器相关
    if (slPlayer->prepare(global) < 0) {
        if (callJava) {
            callJava->onError(Thread_child, -1, "创建OpenSL ES失败");
        }
        return;
    }

    glPlayer = new GLPlayer(ffDecoder->videoDecCtx);
    if (glPlayer->prepare(global) < 0) {
        if (callJava) {
            callJava->onError(Thread_child, -1, "创建OpenGL ES失败");
        }
        return;
    }

    if (callJava) {
        global->status = Prepared;
        callJava->onPrepare(Thread_child);
    }
}


void Player::setDataSource(const char *src, ANativeWindow *window, JNIEnv *env) {
    this->src = src;
    this->window = window;
    this->env = env;
    pthread_create(&pidPrepare, nullptr, preparedThread, this);
}

Player::Player() {
    //初始化全局参数
    global = new Global();
}

Player::~Player() {
    if (callJava) {
        delete callJava;
        callJava = nullptr;
    }

    if (global) {
        delete global;
        global = nullptr;
    }
    if (slPlayer) {
        delete slPlayer;
        slPlayer = nullptr;
    }
}

long Player::getCurrentPosition() {
    return 0;
}

long Player::getDuration() {
    if (ffDecoder && ffDecoder->iFmtCtx) {
        return ffDecoder->iFmtCtx->duration;
    }
    return 0;
}

unsigned int Player::getHeight() {//可能只有音频流
    if (ffDecoder && ffDecoder->videoDecCtx) {
        return ffDecoder->videoDecCtx->height;
    }
    return 0;
}

unsigned int Player::getWidth() {//可能只有音频流
    if (ffDecoder && ffDecoder->videoDecCtx) {
        return ffDecoder->videoDecCtx->width;
    }
    return 0;
}

int Player::start() {
    int ret = -1;
    if (!ffDecoder) {//解码器都没有创建成功
        LOGE("check your #prepare method! ");
        return ret;
    }

    if (global->status != Prepared) {
        LOGE("current status is %d, not Prepared! ", global->status);
        return ret;
    }

    global->status = Playing;
    global->setAllQueueWork(true);
    ffDecoder->decodeThread();
    slPlayer->play();
    glPlayer->play();

    return 0;
}

void Player::initPlayerGlobal(ANativeWindow *window, JNIEnv *env) {
    //存在音频流时，设置音频全局参数
    if (ffDecoder->audioDecCtx) {
        global->fmtType = ffDecoder->outFmtType;
    }

    global->env = env;
    global->window = window;
    if (ffDecoder->videoDecCtx) {
        global->audioTimeBase = ffDecoder->iFmtCtx->streams[ffDecoder->audioIndex]->time_base;
        global->videoTimeBase = ffDecoder->iFmtCtx->streams[ffDecoder->videoIndex]->time_base;
        global->videoFrameSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24,
                                                          ffDecoder->videoDecCtx->width,
                                                          ffDecoder->videoDecCtx->height, 1);
        global->width = ffDecoder->videoDecCtx->width;
        global->height = ffDecoder->videoDecCtx->height;
        global->fps = av_q2d(ffDecoder->iFmtCtx->streams[ffDecoder->videoIndex]->avg_frame_rate);
    }
}
