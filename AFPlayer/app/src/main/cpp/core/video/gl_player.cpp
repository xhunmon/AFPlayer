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
    /*int ret = -1;
    if (!initEgl()) return ret;

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
            "#version 300 es\n"
            "layout (location = 0) in vec3 position;\n"
            "layout (location = 1) in vec3 color;\n"
            "layout (location = 2) in vec2 texCoord;\n"
            "out vec3 ourColor;\n"
            "out vec2 TexCoord;\n"
            "void main(){\n"
            "    gl_Position = vec4(position, 1.0f);\n"
            "    ourColor = color;\n"
            "    TexCoord = texCoord;\n"
            "}\n";

    //precision mediump float; 是从报错日志查出来的
    const GLchar *FRAGMENT_SHADER =
            "#version 300 es\n"
            "precision mediump float;\n"
            "in vec3 ourColor;\n"
            "in vec2 TexCoord;\n"
            "out vec4 color;\n"
            "uniform sampler2D ourTexture;\n"
            "void main(){\n"
            "    color = texture(ourTexture, TexCoord);\n"
            "}\n";
    // Define the viewport dimensions
    //此处应该感觉屏幕实际大小进行处理
    // 绘制区域的宽高
    int32_t windowWidth = ANativeWindow_getWidth(global->window);
    int32_t windowHeight = ANativeWindow_getHeight(global->window);

    double scale = 1.0 * global->width / windowWidth;
    GLsizei showHeight = 1.0 * global->height / scale;
    GLint startY = (windowHeight - showHeight) / 2;
    glViewport(0, startY, windowWidth, showHeight);



    // Build and compile our shader program
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VERTEX_SHADER, nullptr);
    glCompileShader(vertexShader);

    //以下为异常情况处理，打印编译错误（如果有的话）
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        LOGE("ERROR::SHADER::VERTEX::COMPILATION_FAILED1 %s\n", infoLog);
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER, nullptr);
    glCompileShader(fragmentShader);

    //以下为异常情况处理，打印编译错误（如果有的话）
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        LOGE("ERROR::SHADER::VERTEX::COMPILATION_FAILED2 %s\n", infoLog);
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // Set up vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
            //     ---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -
            1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 右上
            1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // 右下
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // 左下
            -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f  // 左上
    };
    //从0开始，画两个三角形，组成矩形（从0开始，画两个三角形的索引即可）
    GLuint indices[] = {  // Note that we start from 0!
            0, 1, 3, // First Triangle
            1, 2, 3  // Second Triangle
    };

    // 1. 绑定顶点数组对象
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *) nullptr);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // TexCoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid *) (6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // Unbind VAO


    // Load and create a texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D,
                  texture); // All upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // Set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT);    // Set texture wrapping to GL_REPEAT (usually basic wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load image, create texture and generate mipmaps
    return true;
}

GLPlayer::~GLPlayer() {
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
    }
    if (VBO) {
        glDeleteBuffers(1, &VBO);
    }
    if (EBO) {
        glDeleteBuffers(1, &EBO);
    }
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
    glPlayer->play();
}

int GLPlayer::start() {
// 使用智能指针，线程结束时，自动删除本类指针
    global->env->GetJavaVM(&javaVm);
    std::shared_ptr<GLPlayer> that(this);
    std::thread t(playThread, that);
    t.detach();
    return 0;
}

void GLPlayer::play() {
    if (!initEgl()) return;

    if (!initOpenGL()) return;
//    remove("/storage/emulated/0/Download/Kobe.rgb24");
//    FILE *outFile = fopen("/storage/emulated/0/Download/Kobe.rgb24", "wb+");
    auto *buffer = new unsigned char[global->videoFrameSize];
    AVFrame *frame;
    //就让他一直显示吧
    while (global->status == Started) {
        if (global->videoFrameQ->pop(frame)) {
            memcpy(buffer, frame->data[0], global->videoFrameSize);
            //获得 当前这一个画面 播放的相对的时间
            double clock = frame->pts * av_q2d(global->videoTimeBase);
            double audioClock = global->audioClock;
            LOGD("loading video pts: %5lld,video clock: %5lf,audio clock: %5lf", frame->pts, clock,
                 audioClock);
            if (clock != 0) {
                //比较音频与视频
                //间隔 音视频相差的间隔
                double diff = clock - audioClock;
                if (diff > 0) {
                    LOGE("视频快了：%lf", diff);
                    av_usleep(diff * 1000000);
                } else if (diff < 0) {
                    LOGE("音频快了：%lf", diff);
                    if (fabs(diff) >= 0.05) {
                        av_frame_free(&frame);
                        continue;//丢包
                    } else {
                        //需要快点赶上音频
                    }
                }
            }
//            fwrite(buffer, 1, global->videoFrameSize, outFile);
            av_frame_free(&frame);
        }
        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, global->width, global->height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, buffer);

        // Activate shader
        glUseProgram(shaderProgram);

        // Draw container
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        eglSwapBuffers(display, surface);
    }

//    fclose(outFile);
}
