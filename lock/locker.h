#ifndef LOCKER_H
#define LOCKER_H

// 该类用于线程间同步

#include <exception>
#include <pthread.h>
#include <semaphore.h>

// 信号量类
class sem{
public:
    sem(){
        // 初始化一个未命名的信号量，第二个变量代表该信号量为当前进程使用
        if(sem_init(&m_sem, 0, 0) != 0){
            throw std::exception();
        }
    }
    ~sem(){
        sem_destroy(&m_sem);
    }

    // 信号量减1,成功减1会返回0
    bool wait(){
        return sem_wait(&m_sem) == 0;
    }

    // 信号量加1，成功加1会返回0
    bool post(){
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

// 互斥锁
class locker{
private:
    pthread_mutex_t m_mutex;
public:
    locker(){
        if(pthread_mutex_init(&m_mutex, NULL) != 0){
            throw std::exception();
        }
    }
    ~locker(){
        pthread_mutex_destroy(&m_mutex);
    }

    bool get_lock(){
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    bool release_lock(){
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
};

// 条件变量
class cond{
private:
    pthread_cond_t m_cond;
    pthread_mutex_t m_mutex;
public:
    cond(){
        if(pthread_mutex_init(&m_mutex, NULL) != 0){
            throw std::exception();
        }
        if(pthread_cond_init(&m_cond, NULL) != 0){
            pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond(){
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }

    // 等待条件变量
    bool wait(){
        int ret = 0;
        pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, &m_mutex);
        pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }

    // 唤醒等待条件变量的线程
    bool signale(){
        return pthread_cond_signal(&m_cond) == 0;
    }
};

#endif