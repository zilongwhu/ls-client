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

#include <pthread.h>
#include "net_poller.h"
#include "client_epex.h"

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

        int init(int wn);
        int run();
        void stop();
    private:
        int _worker_num;
        pthread_t *_tids;
        ClientEpex *_workers;
};

#endif
