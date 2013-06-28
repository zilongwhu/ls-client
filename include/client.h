/*
 * =====================================================================================
 *
 *       Filename:  client.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年06月28日 21时14分23秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#ifndef __LS_CLIENT_H__
#define __LS_CLIENT_H__

#include "exnet.h"
#include "dlist.h"
#include <pthread.h>

struct NetStub
{
    int _sock;
    int _status;
    int _errno;

    net_head_t _req_head;
    net_head_t _res_head;

    void *_req_buf;
    void *_res_buf;

    int _req_len;
    int _res_len;

    int _timeout;
    struct timeval _start_tm;
    struct timeval _done_tm;
    struct
    {
        int _total;
        int _send_head;
        int _send_body;
        int _recv_head;
        int _recv_body;
    } _tm;

    __dlist_t _link;

    NetStub()
    {
        _sock = -1;
        _status = 0;
        _errno = 0;
        ::bzero(&_req_head, sizeof _req_head);
        ::bzero(&_res_head, sizeof _res_head);
        _req_buf = _res_buf = NULL;
        _req_len = _res_len = 0;
        _timeout = -1;
        ::bzero(&_start_tm, sizeof _start_tm);
        ::bzero(&_done_tm, sizeof _done_tm);
        ::bzero(&_tm, sizeof _tm);
        DLIST_INIT(&_link);
    }
};

class Mutex
{
    private:
        Mutex(const Mutex &);
        Mutex &operator =(const Mutex &);
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

class ClientEpex
{
    private:
        ClientEpex(const ClientEpex &);
        ClientEpex &operator =(const ClientEpex &);
    public:
        ClientEpex() { _epex = NULL; }
        ~ClientEpex() { epex_close(_epex); _epex = NULL; }

        int init(int size = 1024)
        {
            if (_epex)
                return 0;
            _epex = epex_open(size);
            return _epex ? 0 : -1;
        }

        void attach(NetStub *st)
        {
            AutoLock __lock(_mutex);
            DLIST_INSERT_B(&st->_link, &_attach_list);
        }
        void detach(NetStub *st)
        {
            AutoLock __lock(_mutex);
            if (!DLIST_EMPTY(&st->_link))
            {
                DLIST_REMOVE(&st->_link);
                return ;
            }
            DLIST_INSERT_B(&st->_link, &_detach_list);
        }

        void run() { }
    private:
        Mutex _mutex;
        __dlist_t _attach_list;
        __dlist_t _detach_list;

        epex_t _epex;
};

#endif
