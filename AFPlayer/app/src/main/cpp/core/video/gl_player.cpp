/**
 * description: 使用OpenSL ES进行声音播放 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/18
 */


#include "gl_player.h"

int GLPlayer::prepare(Global *g) {
    this->global = g;
    int ret = -1;
    //初始化等需要跟draw时线程一致，否者无法播放
    /* if (!initEgl()) return ret;

     if (!initOpenGL()) return ret;*/

    return 0;
}

bool GLPlayer::initEgl() {
    bool ret = false;
    // 1 EGL dispaly创建和初始化
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed");
        return ret;
    }
    //初始化这个显示设备，该方法会返回一个布尔型变量来代表执行状态，后面两个参数则代表Major和Minor的版本。
    if (EGL_FALSE == eglInitialize(display, nullptr, nullptr)) {
        LOGE("NativeEngine: failed to init display, error %d", eglGetError());
        return ret;
    }

    //一旦EGL有了Display之后，它就可以将OpenGL ES的输出和设备的屏幕桥接起来，但是需要指定一些配置项，
    // 类似于色彩格式、像素格式、RGBA的表示以及SurfaceType等
    //2 surface窗口配置
    EGLConfig config;
    EGLint configNum;
    EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BUFFER_SIZE, 24,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
    };
    if (EGL_TRUE != eglChooseConfig(display, attribs, &config, 1, &configNum)) {
        LOGE("eglChooseConfig failed!");
        return ret;
    }

    //3. 创建surface，将EGL和设备的屏幕连接起来
    EGLint format;
    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID,
                            &format)) {
        LOGE("eglGetConfigAttrib() returned error %d", eglGetError());
        return ret;
    }
    ANativeWindow_setBuffersGeometry(global->window, 0, 0, format);
    //注意：surface创建的申请线程需要与 eglSwapBuffers 保持一致
    if (!(surface = eglCreateWindowSurface(display, config, global->window, nullptr))) {
        LOGE("eglCreateWindowSurface() returned error %d", eglGetError());
    }


    //4 context 创建关联的上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed!");
        return ret;
    }

    //5. 进行绑定
    if (EGL_TRUE != eglMakeCurrent(display, surface, surface, context)) {
        LOGE("eglMakeCurrent failed！");
        return ret;
    }

    LOGD("EGL init success!");
    return true;
}

bool GLPlayer::initOpenGL() {
    // 版本查看NDK Demo
    const GLchar *VERTEX_SHADER =
            "attribute vec4 av_Position;\n"
            "attribute vec2 af_Position;\n"
            "varying vec2 v_texPosition;\n"
            "void main() {\n"
            "    v_texPosition = af_Position;\n"
            "    gl_Position = av_Position;\n"
            "}";

    //precision mediump float; 是从报错日志查出来的
    const GLchar *FRAGMENT_SHADER =
            "precision mediump float;\n"
            "varying vec2 v_texPosition;\n"
            "uniform sampler2D sampler_y;\n"
            "uniform sampler2D sampler_u;\n"
            "uniform sampler2D sampler_v;\n"
            "void main() {\n"
            "    float y,u,v;\n"
            "    y = texture2D(sampler_y,v_texPosition).r;\n"
            "    u = texture2D(sampler_u,v_texPosition).r- 0.5;\n"
            "    v = texture2D(sampler_v,v_texPosition).r- 0.5;\n"
            "\n"
            "    vec3 rgb;\n"
            "//    yuv---> rgb     固定代码\n"
            "    rgb.r = y + 1.403 * v;\n"
            "    rgb.g = y - 0.344 * u - 0.714 * v;\n"
            "    rgb.b = y + 1.770 * u;\n"
            "\n"
            "    gl_FragColor = vec4(rgb,1);\n"
            "}";
    // Define the viewport dimensions
    //此处应该感觉屏幕实际大小进行处理
    // 绘制区域的宽高
    int32_t windowWidth = ANativeWindow_getWidth(global->window);
    int32_t windowHeight = ANativeWindow_getHeight(global->window);

    double scale = 1.0 * global->width / windowWidth;
    GLsizei showHeight = 1.0 * global->height / scale;
    GLint startY = (windowHeight - showHeight) / 2;
    glViewport(0, startY, windowWidth, showHeight);

    // Set up vertex data (and buffer(s)) and attribute pointers
    vertexData = new GLfloat[]{
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f
    };
    textureData = new GLfloat[]{
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
    };

//    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, VERTEX_SHADER);
    shaderProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    avPosition_yuv = glGetAttribLocation(shaderProgram, "av_Position");
    afPosition_yuv = glGetAttribLocation(shaderProgram, "af_Position");

    sampler_y = glGetUniformLocation(shaderProgram, "sampler_y");
    sampler_u = glGetUniformLocation(shaderProgram, "sampler_u");
    sampler_v = glGetUniformLocation(shaderProgram, "sampler_v");
    textureId_yuv = new GLuint[]{0, 0, 0};
    glGenTextures(3, textureId_yuv);
    for (int i = 0; i < 3; i++) {
        glBindTexture(GL_TEXTURE_2D, textureId_yuv[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    return true;
}

GLPlayer::~GLPlayer() {
    if (javaVm) {
        javaVm->DetachCurrentThread();
    }
}

static void playThread(std::shared_ptr<GLPlayer> glPlayer) {
    //将线程附加到虚拟机，并获取env
    JNIEnv *env;
    if (glPlayer->javaVm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        LOGE("Fail to attach thread!");
        return;
    }
    glPlayer->playVideo();
}

void GLPlayer::playVideo() {
    //初始化和播放要同一个线程，不然会黑屏
    if (!initEgl()) return;
    if (!initOpenGL()) return;
//    remove("/storage/emulated/0/Download/test.yuv420p");
//    FILE *outFile = fopen("/storage/emulated/0/Download/test.yuv420p", "wb+");
    //就让他一直显示吧
    while (global->status == Playing) {
        AVPacket *avPacket = av_packet_alloc();
        if (!global->videoPktQ->pop(avPacket)) {
            releasePktCallback(&avPacket);
            continue;
        }
//        视频解码 比较耗时  多线程环境
        pthread_mutex_lock(&codecMutex);
        //解码操作
        if (avcodec_send_packet(videoDecCtx, avPacket) != 0) {
            releasePktCallback(&avPacket);
            pthread_mutex_unlock(&codecMutex);
            continue;
        }
        AVFrame *avFrame = av_frame_alloc();
        if (avcodec_receive_frame(videoDecCtx, avFrame) != 0) {
            releaseFrameCallback(&avFrame);
            releasePktCallback(&avPacket);
            pthread_mutex_unlock(&codecMutex);
            continue;
        }

        av_usleep(getDelayTime(*avFrame) * 1000000);
        if (avFrame->format == AV_PIX_FMT_YUV420P) {
            /*
             * 比如704*576分辨率的视频，它的width=704,height=576,摄像机芯片一般会要求64或者128对齐，当128位对齐时，
             * 704不能被128整除，需要在每一行结尾补64字节0x00占位，它的linesize也就是每一行的长度768。
             * 所以在解码的时候,宽和yuv的buffer都要以linesize[0]为准 https://blog.csdn.net/jianglei382873012/article/details/104674933/
             */
            drawYUV(avFrame->linesize[0], videoDecCtx->height, avFrame->data[0], avFrame->data[1],
                    avFrame->data[2]);
//            int bufLen = avFrame->linesize[0] * videoDecCtx->height;
//            fwrite(avFrame->data[0], 1, bufLen, outFile);
//            fwrite(avFrame->data[1], 1, bufLen / 4, outFile);
//            fwrite(avFrame->data[2], 1, bufLen / 4, outFile);
        } else {
            AVFrame *pFrameYUV420P = av_frame_alloc();
            int num = av_image_get_buffer_size(
                    AV_PIX_FMT_YUV420P,
                    videoDecCtx->width,
                    videoDecCtx->height,
                    1);
            uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
            av_image_fill_arrays(
                    pFrameYUV420P->data,
                    pFrameYUV420P->linesize,
                    buffer,
                    AV_PIX_FMT_YUV420P,
                    videoDecCtx->width,
                    videoDecCtx->height,
                    1);
            SwsContext *sws_ctx = sws_getContext(
                    videoDecCtx->width,
                    videoDecCtx->height,
                    videoDecCtx->pix_fmt,
                    videoDecCtx->width,
                    videoDecCtx->height,
                    AV_PIX_FMT_YUV420P,
                    SWS_BICUBIC, nullptr, nullptr, nullptr);

            if (!sws_ctx) {
                releaseFrameCallback(&pFrameYUV420P);
                av_free(buffer);
                pthread_mutex_unlock(&codecMutex);
                continue;
            }
            sws_scale(
                    sws_ctx,
                    reinterpret_cast<const uint8_t *const *>(avFrame->data),
                    avFrame->linesize,
                    0,
                    avFrame->height,
                    pFrameYUV420P->data,
                    pFrameYUV420P->linesize);
            drawYUV(pFrameYUV420P->linesize[0], videoDecCtx->height, pFrameYUV420P->data[0],
                    pFrameYUV420P->data[1], pFrameYUV420P->data[2]);
//            int bufLen = pFrameYUV420P->linesize[0] * videoDecCtx->height;
//            fwrite(pFrameYUV420P->data[0], 1, bufLen, outFile);
//            fwrite(pFrameYUV420P->data[1], 1, bufLen / 4, outFile);
//            fwrite(pFrameYUV420P->data[2], 1, bufLen / 4, outFile);
            releaseFrameCallback(&pFrameYUV420P);
            av_free(buffer);
            sws_freeContext(sws_ctx);
        }
        releaseFrameCallback(&avFrame);
        releasePktCallback(&avPacket);
        pthread_mutex_unlock(&codecMutex);
    }
}


void GLPlayer::play() {
    // 使用智能指针，线程结束时，自动删除本类指针
    global->env->GetJavaVM(&javaVm);
    std::shared_ptr<GLPlayer> that(this);
    std::thread t(playThread, that);
    t.detach();
}

GLPlayer::GLPlayer(AVCodecContext *videoDecCtx) {
    this->videoDecCtx = videoDecCtx;
    pthread_mutex_init(&codecMutex, nullptr);
}

void
GLPlayer::drawYUV(const int width, const int height, const void *y, const void *u, const void *v) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glUseProgram(shaderProgram);

    //将顶点数据拷贝到缓存对象中
    glEnableVertexAttribArray(avPosition_yuv);
    glVertexAttribPointer(avPosition_yuv, 2, GL_FLOAT, GL_FALSE, 8, vertexData);

    glEnableVertexAttribArray(afPosition_yuv);
    glVertexAttribPointer(afPosition_yuv, 2, GL_FLOAT, GL_FALSE, 8, textureData);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId_yuv[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE,
                 GL_UNSIGNED_BYTE, y);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureId_yuv[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, u);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureId_yuv[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, v);

    glUniform1i(sampler_y, 0);
    glUniform1i(sampler_u, 1);
    glUniform1i(sampler_v, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//        LOGE("..........playing...");
    eglSwapBuffers(display, surface);
}

double GLPlayer::getDelayTime(AVFrame &avFrame) {
    //    先获取视频时间戳  处理之后
    double pts = av_frame_get_best_effort_timestamp(&avFrame);
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
//     1.001*40ms
//    pts=pts * time_base.num / time_base.den;
    double audioClock = global->audioClock;
//    double clock = avFrame.pts * av_q2d(global->videoTimeBase);
    double clock = pts * av_q2d(global->videoTimeBase);
    double diff = audioClock - clock;
    //    返回秒数 3ms 以内
    //音频超越视频  3ms   1
    double delayTime = 0;

    //视频超越  音频3ms   2
    if (diff > 0.003) {
//        视频休眠时间
        delayTime = delayTime * 2 / 3;// * 3/2;
        if (delayTime < global->defaultDelayTime / 2) {
//            用户有所察觉
            delayTime = global->defaultDelayTime * 2 / 3;
        } else if (delayTime > global->defaultDelayTime * 2) {
            delayTime = global->defaultDelayTime * 2;
        }
        LOGD("视频播放速度---1----%f", delayTime);
    } else if (diff < -0.003) {
        //视频超前    休眠时间 相比于以前大一些
        delayTime = delayTime * 3 / 2;
        if (delayTime < global->defaultDelayTime / 2) {
            delayTime = global->defaultDelayTime * 2 / 3;
        } else if (delayTime > global->defaultDelayTime * 2) {
            delayTime = global->defaultDelayTime * 2;
        }
        LOGD("视频播放速度----2---%f", delayTime);
    }
    //感觉的 视频加速
    if (diff >= 0.5) {
        delayTime = 0;
        LOGD("视频播放速度----3---%f", delayTime);
    } else if (diff <= -0.5) {
        delayTime = global->defaultDelayTime * 2;
        LOGD("视频播放速度----4---%f", delayTime);
    }
    //    音频太快了   视频怎么赶也赶不上        视频队列全部清空   直接解析最新的 最新鲜的
    if (diff >= 10) {
        global->videoPktQ->clear();
        delayTime = global->defaultDelayTime;
        LOGD("视频播放速度---5----%f", delayTime);
    }
    //视频太快了  音频赶不上
    if (diff <= -10) {
        global->audioPktQ->clear();
        delayTime = global->defaultDelayTime;
        LOGD("视频播放速度----6---%f", delayTime);
    }
    return delayTime;
}