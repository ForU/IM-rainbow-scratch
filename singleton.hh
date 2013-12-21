#ifndef INCLUDE_SINGLETON_HPP
#define INCLUDE_SINGLETON_HPP

#include <pthread.h>            // pthread_once
#include <stdlib.h>             // atexit

template <typename T>
class SingletonX
{
public:
    static T& instance() {
        pthread_once(&s_once, init);
        return *s_data;
    }

private:
    SingletonX() {}
    ~SingletonX() {}

    static void init() {
        s_data = new T();
        ::atexit(destroy);
    }
    static void destroy() {
        delete s_data;
    }

private:
    static T* s_data;
    static pthread_once_t s_once;
};

template<typename T> pthread_once_t SingletonX<T>::s_once = PTHREAD_ONCE_INIT;
template<typename T> T* SingletonX<T>::s_data = NULL;

#endif /* INCLUDE_SINGLETON_HPP */
