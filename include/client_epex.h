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

class NetStub;
class ClientEpex
{
    private:
        ClientEpex(const ClientEpex &);
        ClientEpex &operator =(const ClientEpex &);
    public:
        ClientEpex()
        {
            _epex = NULL;
            DLIST_INIT(&_attach_list);
            DLIST_INIT(&_detach_list);
        }

        ~ClientEpex()
        {
            epex_close(_epex);
            _epex = NULL;
            __dlist_t *list;
            while (!DLIST_EMPTY(&_attach_list))
            {
                list = DLIST_NEXT(&_attach_list);
                DLIST_REMOVE(list);
            }
            while (!DLIST_EMPTY(&_detach_list))
            {
                list = DLIST_NEXT(&_detach_list);
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

        void run();
    private:
        Mutex _mutex;
        __dlist_t _attach_list;
        __dlist_t _detach_list;

        epex_t _epex;
};

#endif
