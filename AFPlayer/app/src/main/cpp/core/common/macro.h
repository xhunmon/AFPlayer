/**
 * description: 一些公共的宏参数、结构体、错误代码等信息 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/7
 */
#ifndef AFPLAYER_MACRO_H
#define AFPLAYER_MACRO_H

enum ThreadType {
    Thread_main, //主线中
    Thread_child //子线程中
};

enum Status {
    None,
    Initialized,
    Prepared,
    Started,
    Paused,
    Stopped,
    Released,
    Error
};

//#define DEFAULT_MAX_VIDEOQ_SIZE (5 * 256 * 1024) //默认视频队列大小
//#define DEFAULT_MAX_AUDIOQ_SIZE (5 * 16 * 1024)  //默认音频队列大小
#define DEFAULT_MAX_VIDEOQ_SIZE (50) //默认视频队列大小
#define DEFAULT_MAX_AUDIOQ_SIZE (50)  //默认音频队列大小

#define ERR_OPEN_FAIL -101 //打开文件失败
#define ERR_NO_STREAM -102 //输入源异常
#define ERR_NO_VIDEO_AND_AUDIO_STREAM -103 //无音频视频流
#define ERR_NO_DECODER -104 //无解码器
#define ERR_ALLOCATE_FAIL -105 //申请内存失败
#define ERR_COPY_DECODER_FAIL -106 //复制解码器参数失败
#define ERR_CREATE_HW_DEVICE -107 //创建硬解码器失败
#define ERR_DECODER_OPEN_FIAL -108 //打开解码器失败


#endif //AFPLAYER_MACRO_H
