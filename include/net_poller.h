/*
 * =====================================================================================
 *
 *       Filename:  net_poller.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/29/2013 02:06:00 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *   Organization:  edu.whu
 *
 * =====================================================================================
 */
#ifndef __LS_CLIENT_NET_POLLER_H__
#define __LS_CLIENT_NET_POLLER_H__

#include "lock.h"
#include "dlist.h"
#include "net_talk.h"

class ClientEpex;
class NetPoller
{
    private:
        NetPoller(const NetPoller &);
        NetPoller &operator =(const NetPoller &);
    public:
        NetPoller()
            : _cond(_mutex)
        {
            _epex = NULL;
            DLIST_INIT(&_avail_list);
            DLIST_INIT(&_list);
        }
        ~ NetPoller() { }

        void setEpex(ClientEpex *epex) { _epex = epex; }

        void add(NetTalk *tt);
        void cancel(NetTalk *tt);
        void cancelAll();
        void done(NetStub *st);
        int poll(NetTalk **talks, int count, int timeout_ms);
    private:
        Mutex _mutex;
        Cond _cond;
        __dlist_t _avail_list;
        __dlist_t _list;
        ClientEpex *_epex;
};

#endif
