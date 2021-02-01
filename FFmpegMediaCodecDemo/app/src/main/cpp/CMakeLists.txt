##############################################################################
####   避坑：项目build.gradle版本gradle:4.0.0不能生成动态库，使用3.4.1正常     ######
#############################################################################

#1、添加版本
cmake_minimum_required(VERSION 3.4.1)

include_directories(include)
file(GLOB core_srcs ${CMAKE_SOURCE_DIR}/core/*.cpp)

#2、编译库，把native-lib.cpp编译成以native-lib为名称的动态库
add_library(
        native-lib
        SHARED
        player_main.cpp ${core_srcs})

#3、设置C++编译参数（CMAKE_CXX_FLAGS是全局变量）
#添加其他预编译库还可以使用这种方式
#使用-L指导编译时库文件的查找路径
#cxx_flags "cxx_flags -L目录"
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")

#github.com/android/ndk-samples/-/blob/endless-tunnel/app/src/main/cpp/CMakeLists.tx
#4、链接到库文件，jni/c/c++可以引入链接到
target_link_libraries(
        native-lib
        log
        # 先把有依赖的库，先依赖进来；注意顺序。
        avformat avcodec avfilter avutil swresample swscale
        z
)