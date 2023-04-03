/**
 * description: 使用OpenSL ES进行声音播放 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/18
 */

#ifndef AFPLAYER_GL_PLAYER_H
#define AFPLAYER_GL_PLAYER_H


#include "../common/global.h"
#include "gl_helper.cpp"
#include <GLES2/gl2.h>
//#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <unistd.h>
#include <thread>

extern "C" {
#include "libavutil/imgutils.h"
#include <libswscale/swscale.h>
};

class GLPlayer {
private:
    EGLDisplay display;
    EGLSurface surface = nullptr;
    GLuint shaderProgram;
    GLuint avPosition_yuv;
    GLuint afPosition_yuv;
    GLint sampler_y;
    GLint sampler_u;
    GLint sampler_v;
    GLuint *textureId_yuv;
    GLfloat *vertexData;
    GLfloat *textureData;

public:
    Global *global = nullptr;
    JavaVM *javaVm;
    pthread_mutex_t codecMutex;
    AVCodecContext *videoDecCtx = nullptr;

    GLPlayer(AVCodecContext *videoDecCtx);

    ~GLPlayer();

    int prepare(Global *g);

    void playVideo();

    void drawYUV(const int width, const int height, const void *y, const void *u, const void *v);

    bool initEgl();

    bool initOpenGL();

    void play();

    double getDelayTime(AVFrame &avFrame);
};


#endif //AFPLAYER_GL_PLAYER_H
