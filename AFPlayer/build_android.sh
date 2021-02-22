#!/bin/bash

API=21
NDK=/Users/Qincji/Desktop/develop/android/source/sdk/ndk/android-ndk-r21
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64

function build_android() {
  ./configure \
    --prefix=$PREFIX \
    --disable-opencl \
    --disable-doc \
    --disable-everything \
    --disable-htmlpages \
    --disable-podpages \
    --disable-debug \
    --disable-programs \
    --disable-demuxers \
    --disable-muxers \
    --disable-decoders \
    --disable-encoders \
    --disable-bsfs \
    --disable-indevs \
    --disable-outdevs \
    --disable-filters \
    --disable-protocols \
    --disable-hwaccels \
    --disable-avdevice \
    --disable-postproc \
    --disable-devices \
    --disable-symver \
    --disable-stripping \
    --disable-asm \
    --disable-w32threads \
    --disable-parsers \
    --disable-shared \
    --enable-static \
    --enable-swscale \
    --enable-protocol=file \
    --enable-protocol=hls \
    --enable-protocol=rtmp \
    --enable-protocol=http \
    --enable-protocol=tls \
    --enable-protocol=https \
    --enable-demuxer=aac \
    --enable-demuxer=h264 \
    --enable-demuxer=mov \
    --enable-demuxer=flv \
    --enable-demuxer=avi \
    --enable-demuxer=hevc \
    --enable-demuxer=gif \
    --enable-parser=h264 \
    --enable-parser=hevc \
    --enable-parser=aac \
    --enable-decoder=aac \
    --enable-decoder=h264 \
    --enable-decoder=flv \
    --enable-decoder=gif \
    --enable-decoder=mpeg4 \
    --enable-bsf=aac_adtstoasc \
    --enable-bsf=h264_mp4toannexb \
    --enable-jni \
    --enable-mediacodec \
    --enable-decoder=h264_mediacodec \
    --enable-decoder=hevc_mediacodec \
    --enable-decoder=mpeg4_mediacodec \
    --enable-hwaccel=h264_mediacodec \
    --enable-neon \
    --enable-gpl \
    --cross-prefix=$CROSS_PREFIX \
    --target-os=android \
    --arch=$ARCH \
    --cpu=$CPU \
    --cc=$CC \
    --cxx=$CXX \
    --enable-cross-compile \
    --sysroot=$SYSROOT \
    --extra-cflags="-Os -fpic $OPTIMIZE_CFLAGS" \
    --extra-ldflags="$ADDI_LDFLAGS" || exit 1
  make clean
  make -j8
  make install
}


#armv8-a
ARCH=arm64
CPU=armv8-a
CC=$TOOLCHAIN/bin/aarch64-linux-android$API-clang
CXX=$TOOLCHAIN/bin/aarch64-linux-android$API-clang++
SYSROOT=$NDK/toolchains/llvm/prebuilt/darwin-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=$CPU"
#build_android

#armv7-a
ARCH=arm
CPU=armv7-a
CC=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang
CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang++
SYSROOT=$NDK/toolchains/llvm/prebuilt/darwin-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU "
build_android

#x86
ARCH=x86
CPU=x86
CC=$TOOLCHAIN/bin/i686-linux-android$API-clang
CXX=$TOOLCHAIN/bin/i686-linux-android$API-clang++
SYSROOT=$NDK/toolchains/llvm/prebuilt/darwin-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/i686-linux-android-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"
#build_android

#x86_64
ARCH=x86_64
CPU=x86-64
CC=$TOOLCHAIN/bin/x86_64-linux-android$API-clang
CXX=$TOOLCHAIN/bin/x86_64-linux-android$API-clang++
SYSROOT=$NDK/toolchains/llvm/prebuilt/darwin-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/x86_64-linux-android-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=$CPU -msse4.2 -mpopcnt -m64 -mtune=intel"
#build_android
