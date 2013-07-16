/*
 * =====================================================================================
 *
 *       Filename:  lock.h
 *
 *    Description:  
 *
 *
 *        Version:  1.0
 *        Created:  06/29/2013 02:04:05 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *   Organization:  edu.whu
 *
 * =====================================================================================
 */
#ifndef __LS_CLIENT_LOCK_H__
#define __LS_CLIENT_LOCK_H__

#include <pthread.h>
#include <sys/time.h>

class Cond;
class Mutex
{
    private:
        Mutex(const Mutex &);
        Mutex &operator =(const Mutex &);
        friend class Cond;
    public:
        Mutex()
        {
            pthread_mutex_init(&_mutex, NULL);
        }
        ~Mutex()
        {
            pthread_mutex_destroy(&_mutex);
        }
        bool lock()
        {
            return pthread_mutex_lock(&_mutex) == 0;
        }
        void unlock()
        {
            pthread_mutex_unlock(&_mutex);
        }
    private:
        pthread_mutex_t _mutex;
};

class Cond
{
    private:
        Cond(const Cond &);
        Cond &operator =(const Cond &);
    public:
        Cond(Mutex &mutex)
            : _mutex(mutex)
        {
            pthread_cond_init(&_cond, NULL);
        }
        ~Cond()
        {
            pthread_cond_destroy(&_cond);
        }
        int wait(int timeout_ms)
        {
            if (timeout_ms < 0)
                return pthread_cond_wait(&_cond, &_mutex._mutex);
            struct timeval tv;
            gettimeofday(&tv, NULL);
            struct timespec ts;
            ts.tv_sec = tv.tv_sec;
            ts.tv_nsec = tv.tv_usec * (long)1000 + timeout_ms * (long)1000000;
            if (ts.tv_nsec > 1000000000l)
            {
                ts.tv_sec += ts.tv_nsec/1000000000l;
                ts.tv_nsec %= 1000000000l;
            }
            return pthread_cond_timedwait(&_cond, &_mutex._mutex, &ts);
        }
        int signal() { return pthread_cond_signal(&_cond); }
        int broadcast() { return pthread_cond_broadcast(&_cond); }
    private:
        Mutex &_mutex;
        pthread_cond_t _cond;
};

class AutoLock
{
    private:
        AutoLock(const AutoLock &);
        AutoLock &operator =(const AutoLock &);
    public:
        AutoLock(Mutex &mutex)
            :_mutex(mutex)
        {
            _mutex.lock();
        }
        ~AutoLock() { _mutex.unlock(); }
    private:
        Mutex &_mutex;
};

#endif
