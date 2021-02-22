/**
 * description: 线程安全的队列 <br>
 *
 * @author 秦城季
 * @email xhunmon@126.com
 * @blog https://qincji.gitee.io
 * @date 2021/2/8
 */

#ifndef AFPLAYER_SAFE_QUEUE_H
#define AFPLAYER_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>


using namespace std;

template<typename T>
class SafeQueue {
    typedef void (*ReleaseCallback)(T *);//释放接口回调

    typedef void (*SyncHandle)(queue<T> &);//同步处理方法

private:
    queue<T> que;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    ReleaseCallback releaseCallback;
    SyncHandle syncHandle;
    bool isWork = false;
    uint32_t maxSize;
public:
    SafeQueue(uint32_t maxSize) {
        this->maxSize = maxSize;
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
    }

    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    bool push(T data) {
        bool ret = true;
        pthread_mutex_lock(&mutex);
        while (isWork && que.size() >= maxSize) {//队列已满
            LOGD("queue full，wait to %s pop...",typeid(T).name());
            pthread_cond_wait(&cond, &mutex);
        }
        if (isWork) {
            que.push(data);
        } else {
            if (releaseCallback) {//这个地方如果用户没有实现，可能会造成内存泄露
                releaseCallback(&data);
            }
            ret = false;
        }
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    bool pop(T &data) {
        bool ret = true;
        pthread_mutex_lock(&mutex);
        while (que.empty() && isWork) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (que.empty() && isWork) {
            ret = false;
        } else {
            data = que.front();
            que.pop();
        }
        pthread_cond_signal(&cond);//当队列已满时，取出成功后通知push
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    bool empty() {
        return que.empty();
    }

    int size() {
        return que.size();
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        int size = que.size();
        for (int i = 0; i < size; ++i) {
            T value = que.front();
            releaseCallback(&value);//必须要传递出去进行内存释放
            que.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    void setWork(bool work) {
        pthread_mutex_lock(&mutex);
        this->isWork = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback r) {
        releaseCallback = r;
    }

    void setSyncHandle(SyncHandle s) {
        syncHandle = s;
    }

    void sync() {
        pthread_mutex_lock(&mutex);
        //同步代码块 当我们调用sync方法的时候，能够保证是在同步块中操作queue 队列
        syncHandle(que);
        pthread_mutex_unlock(&mutex);
    }
};

#endif //AFPLAYER_SAFE_QUEUE_H
