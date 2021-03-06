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

#include <stdlib.h>
#include <pthread.h>
#include "net_stub.h"
#include "net_proxy.h"
#include "net_poller.h"

class Client
{
    private:
        Client(const Client &);
        Client &operator =(const Client &);
    public:
        Client()
        {
            _worker_num = 0;
            _tids = NULL;
            _workers = NULL;
        }
        ~ Client()
        {
            if (_tids)
            {
                delete [] _tids;
                _tids = NULL;
            }
            if (_workers)
            {
                delete [] _workers;
                _workers = NULL;
            }
            _worker_num = 0;
        }

        int init(int wn = 1);
        int run();
        void join();
        void stop();

        void attach(NetStub *st)
        {
            if (st)
            {
                st->_idx = rand()%_worker_num;
                _workers[st->_idx].attach(st);
            }
        }
        void detach(NetStub *st)
        {
            if (st)
            {
                _workers[st->_idx].detach(st);
                st->_idx = 0;
            }
        }
    private:
        int _worker_num;
        pthread_t *_tids;
        NetProxy *_workers;
};

#endif
