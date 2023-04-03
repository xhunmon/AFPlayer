/**
 * description:   <br>
 * @author cxh
 * @date 2023/2/6
 */

#include <GLES2/gl2.h>
//#include <GLES3/gl32.h>
#include "../common/android_log.h"

static GLuint loadShader(int shaderType, const GLchar *source) {
//    GLuint shader = glCreateShader(shaderType);
//    glShaderSource(shader, 1, &source, nullptr);
//    glCompileShader(shader);

    // Build and compile our shader program
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    //以下为异常情况处理，打印编译错误（如果有的话）
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LOGE("ERROR::SHADER::%d::COMPILATION_FAILED1 %s\n", shaderType, infoLog);
    }
    return shader;
}

static GLuint createProgram(const GLchar *vertexSource, const GLchar *fragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
    return shaderProgram;
}
