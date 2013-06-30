/*
 * =====================================================================================
 *
 *       Filename:  client_epex.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/29/2013 02:10:25 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *   Organization:  edu.whu
 *
 * =====================================================================================
 */
#ifndef __LS_CLIENT_EPEX_H__
#define __LS_CLIENT_EPEX_H__

#include "exnet.h"
#include "dlist.h"
#include "lock.h"
#include "net_stub.h"

class ClientEpex
{
    private:
        ClientEpex(const ClientEpex &);
        ClientEpex &operator =(const ClientEpex &);
    public:
        ClientEpex()
        {
            _epex = NULL;
            _stop = false;
            DLIST_INIT(&_attach_list);
        }

        ~ClientEpex()
        {
            epex_close(_epex);
            _epex = NULL;
            _stop = true;
            __dlist_t *list;
            while (!DLIST_EMPTY(&_attach_list))
            {
                list = DLIST_NEXT(&_attach_list);
                DLIST_REMOVE(list);
            }
        }

        int init(int size = 1024)
        {
            if (_epex)
                return 0;
            _epex = epex_open(size);
            return _epex ? 0 : -1;
        }

        void attach(NetStub *st);
        void detach(NetStub *st);

        void stop() { _stop = true; }
        void run();
    private:
        void done(NetStub *st, struct timeval *now = NULL);
    private:
        bool _stop;
        Mutex _mutex;
        __dlist_t _attach_list;

        epex_t _epex;
};

#endif
