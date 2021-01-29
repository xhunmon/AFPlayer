cmake_minimum_required(VERSION 3.17)
project(OpenGLMacTextureDemo)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp)

# 添加头文件
set(GLEW_H /usr/local/Cellar/glew/2.2.0/include/GL)
set(GLFW_H /usr/local/Cellar/glfw/3.3.2/include/GLFW)
include_directories(${GLEW_H} ${GLFW_H})

# 添加目标链接
set(GLEW_LINK /usr/local/Cellar/glew/2.2.0/lib/libGLEW.2.2.dylib)
set(GLFW_LINK /usr/local/Cellar/glfw/3.3.2/lib/libglfw.3.dylib)
link_libraries(${OPENGL} ${GLEW_LINK} ${GLFW_LINK})

add_executable(OpenGLMacTextureDemo ${SOURCE_FILES})

target_link_libraries(
        OpenGLMacTextureDemo
        "-framework OpenGL" #引入OpenGl 使用macos系统的opengl而不是用xcode的
        "-framework GLUT" #引入OpenGl 使用macos系统的opengl而不是用xcode的
)