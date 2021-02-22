##############################################################################
####   避坑：项目build.gradle版本gradle:4.0.0不能生成动态库，使用3.4.1正常     ######
#############################################################################

#1、添加版本
cmake_minimum_required(VERSION 3.4.1)

include_directories(${CMAKE_SOURCE_DIR}/include)

#关闭日志打印
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")

# 添加子目录
add_subdirectory(${CMAKE_SOURCE_DIR}/core)
add_subdirectory(${CMAKE_SOURCE_DIR}/core/common)
add_subdirectory(${CMAKE_SOURCE_DIR}/core/ffmpeg)
add_subdirectory(${CMAKE_SOURCE_DIR}/core/audio)
add_subdirectory(${CMAKE_SOURCE_DIR}/core/video)


#2、编译库，把native-lib.cpp编译成以native-lib为名称的动态库
add_library(
        native-lib
        SHARED
        native_player_jni.cpp)

#3、设置C++编译参数（CMAKE_CXX_FLAGS是全局变量）
#添加其他预编译库还可以使用这种方式
#使用-L指导编译时库文件的查找路径
#cxx_flags "cxx_flags -L目录"
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")

#4、链接到库文件，jni/c/c++可以引入链接到
target_link_libraries(
        native-lib
        core common ffmpeg audio video#引入子目录生成的静态库
        # 先把有依赖的库，先依赖进来；注意顺序。
        avformat avcodec avfilter avutil swresample swscale
        x264.161
        fdk-aac
        ssl.3 crypto.3 #openssl
        OpenSLES    #音频播放
        EGL GLESv3 #视频播放
        log z android
)