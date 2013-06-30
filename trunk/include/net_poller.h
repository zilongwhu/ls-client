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
#include "net_stub.h"

class ClientEpex;
class NetPoller
{
    private:
        NetPoller(const NetPoller &);
        NetPoller &operator =(const NetPoller &);
    public:
        NetPoller(ClientEpex *epex)
            : _cond(_mutex)
        {
            _epex = epex;
            DLIST_INIT(&_avail_list);
            DLIST_INIT(&_list);
        }
        ~ NetPoller();

        void setEpex(ClientEpex *epex) { _epex = epex; }

        bool add(NetTalk *talk, int timeout);
        void cancel(NetTalk *talk);
        void cancelAll();
        int poll(NetTalk **talks, int count, int timeout_ms);
    private:
        int poll(NetTalk *talk);
        void done(NetStub *st);
        friend class ClientEpex;
    private:
        Mutex _mutex;
        Cond _cond;
        __dlist_t _avail_list;
        __dlist_t _list;
        ClientEpex *_epex;
};

#endif
