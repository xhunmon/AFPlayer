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
#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <unistd.h>
#include <thread>

class GLPlayer {
private:
    EGLDisplay display;
    EGLSurface surface = nullptr;
    GLuint VBO, VAO, EBO;
    GLuint texture;
    GLuint shaderProgram;
public:
    Global *global = nullptr;
    JavaVM *javaVm;

    ~GLPlayer();

    int prepare(Global *g);

    int start();

    bool initEgl();

    bool initOpenGL();

    void play();
};


#endif //AFPLAYER_GL_PLAYER_H
