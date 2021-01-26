/**
 * description:   <br>
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/1/26
 */
#ifndef PULLPLAYER_AUDIO_COMMON_H
#define PULLPLAYER_AUDIO_COMMON_H
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <assert.h>
#include <string.h>

#define SLASSERT(x)                   \
  do {                                \
    assert(SL_RESULT_SUCCESS == (x)); \
    (void)(x);                        \
  } while (0)


#endif //PULLPLAYER_AUDIO_COMMON_H
