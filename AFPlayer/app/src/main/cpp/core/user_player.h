/**
 * description: 定义用户操作接口 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/8
 */
#ifndef AFPLAYER_USER_PLAYER_H
#define AFPLAYER_USER_PLAYER_H

class UserPlayer {
public:
    virtual long getCurrentPosition() = 0;

    virtual long getDuration() = 0;

    virtual unsigned int getHeight() = 0;

    virtual unsigned int getWidth() = 0;

    virtual int start() = 0;


    /*

     virtual int pause() = 0;

     virtual int stop() = 0;

     virtual bool isPlaying() = 0;

     virtual int seekTo() = 0;

     virtual int reset() = 0;

     virtual int setVolume(float left, float right) = 0;*/
};

#endif //AFPLAYER_USER_PLAYER_H
