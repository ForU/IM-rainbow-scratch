#ifndef INCLUDE_MUTEX_HPP
#define INCLUDE_MUTEX_HPP

#include <pthread.h>

class PthreadCondition;

class PthreadMutex
{
public:
    PthreadMutex() { pthread_mutex_init(&m_mutex, NULL); }
    ~PthreadMutex() { pthread_mutex_destroy(&m_mutex); }

    void acquire() { pthread_mutex_lock(&m_mutex); }
    void release() { pthread_mutex_unlock(&m_mutex); }
    // true for successfully lock, false for failed to lock
    bool tryAcquire() {
        int rc = pthread_mutex_trylock(&m_mutex);
        return  (0 == rc);
    }

private:
    friend class PthreadCondition;
    pthread_mutex_t m_mutex;
};

class PthreadMutexLock
{
public:
    PthreadMutexLock(PthreadMutex& pthread_mutex)
        : m_mutex_internal(pthread_mutex) {
        m_mutex_internal.acquire();
    }
    ~PthreadMutexLock() {
        m_mutex_internal.release();
    }

private:
    PthreadMutex& m_mutex_internal;
};

class PthreadCondition
{
public:
    PthreadCondition(PthreadMutex& pthread_mutex)
        : m_pthread_mutex(pthread_mutex) {
        pthread_cond_init(&m_cond, NULL);
    }
    ~PthreadCondition() { pthread_cond_destroy(&m_cond); }

    void notify() {
        pthread_cond_signal(&m_cond);
    }
    void notifyAll() {
        pthread_cond_broadcast(&m_cond);
    }
    void wait() {
        pthread_cond_wait(&m_cond, &m_pthread_mutex.m_mutex);
    }
    void waitTimeout(const struct timespec* timeout) {
        pthread_cond_timedwait(&m_cond, &m_pthread_mutex.m_mutex,
                               timeout);
    }

private:
    pthread_cond_t m_cond;
    PthreadMutex& m_pthread_mutex;
};

#endif /* INCLUDE_MUTEX_HPP */
