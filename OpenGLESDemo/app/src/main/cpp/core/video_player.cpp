/**
 * description:   <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/1/30
 */
#include "video_player.h"

VideoPlayer::VideoPlayer(ANativeWindow* window,const char *filename) {


    // 1 EGL dispaly创建和初始化
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(display == EGL_NO_DISPLAY){
        LOGE("eglGetDisplay failed");
        return;
    }
    //初始化这个显示设备，该方法会返回一个布尔型变量来代表执行状态，后面两个参数则代表Major和Minor的版本，
    //比如EGL的版本号是1.0，那么Major将返回1，Minor则返回0。
    if (EGL_FALSE == eglInitialize(display, 0, 0)) {
        LOGE("NativeEngine: failed to init display, error %d", eglGetError());
        return;
    }

    //一旦EGL有了Display之后，它就可
    //以将OpenGL ES的输出和设备的屏幕桥接起来，但是需要指定一些配置
    //项，类似于色彩格式、像素格式、RGBA的表示以及SurfaceType等
    //2 surface窗口配置
    //输出配置
    EGLConfig config;
    EGLint configNum;
    EGLint attribs[] = {
            EGL_SURFACE_TYPE,EGL_WINDOW_BIT,
            EGL_BUFFER_SIZE, 24,
            EGL_RED_SIZE,8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
    };

    if(EGL_TRUE != eglChooseConfig(display, attribs, &config, 1, &configNum)){
        LOGE("eglChooseConfig failed!");
        return;
    }

    //3. 创建surface，将EGL和设备的屏幕连接起来
    EGLSurface surface = NULL;
    EGLint format;
    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID,
                            &format)) {
        LOGE("eglGetConfigAttrib() returned error %d", eglGetError());
        return;
    }
    ANativeWindow_setBuffersGeometry(window, 0, 0, format);
    if (!(surface = eglCreateWindowSurface(display, config, window, 0))) {
        LOGE("eglCreateWindowSurface() returned error %d", eglGetError());
    }


    //4 context 创建关联的上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    EGLContext  context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
    if(context == EGL_NO_CONTEXT){
        LOGE("eglCreateContext failed!");
        return;
    }

    //5. 进行绑定
    if(EGL_TRUE != eglMakeCurrent(display, surface, surface, context)){
        LOGE("eglMakeCurrent failed！");
        return;
    }

    LOGD("NativeEngine: EGL finish, and creating OpenGL.");
    /*********************************************************/
    /*******************       OpenGL     ********************/
    /*********************************************************/


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


    const int pixel_w = 384, pixel_h = 216;
    unsigned char *buffer = new unsigned char[pixel_w * pixel_h * 3];
    FILE *fp = fopen(filename, "rb+");
    fread(buffer, pixel_w * pixel_h * 3, 1, fp);
    // Define the viewport dimensions
    //此处应该感觉屏幕实际大小进行处理
    glViewport(80, 300, pixel_w*2, pixel_h*2);


    // Build and compile our shader program
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VERTEX_SHADER, NULL);
    glCompileShader(vertexShader);

    //以下为异常情况处理，打印编译错误（如果有的话）
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        LOGE("ERROR::SHADER::VERTEX::COMPILATION_FAILED1 %s\n",infoLog);
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER, NULL);
    glCompileShader(fragmentShader);

    //以下为异常情况处理，打印编译错误（如果有的话）
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        LOGE("ERROR::SHADER::VERTEX::COMPILATION_FAILED2 %s\n",infoLog);
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // Set up vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
            //     ---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -
            1.0f,  1.0f,  0.0f,    1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // 右上
            1.0f, -1.0f,  0.0f,    0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // 右下
            -1.0f, -1.0f, 0.0f,    0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 左下
            -1.0f, 1.0f,  0.0f,    1.0f, 1.0f, 0.0f,   0.0f, 0.0f  // 左上
    };
    //从0开始，画两个三角形，组成矩形（从0开始，画两个三角形的索引即可）
    GLuint indices[] = {  // Note that we start from 0!
            0, 1, 3, // First Triangle
            1, 2, 3  // Second Triangle
    };
    GLuint VBO, VAO, EBO;
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *) 0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // TexCoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *) (6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // Unbind VAO


    // Load and create a texture
    GLuint texture;
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

    //就让他一直显示吧
    while (1){
        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pixel_w, pixel_h, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);

        // Activate shader
        glUseProgram(shaderProgram);

        // Draw container
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        eglSwapBuffers(display,surface);
        sleep(0.5);
    }

    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    //省略释放内存等.....
}
