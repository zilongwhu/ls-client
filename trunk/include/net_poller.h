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

#include <vector>
#include "lock.h"
#include "dlist.h"
#include "net_talk.h"
#include "net_stub.h"

class Client;
class NetPoller
{
    private:
        NetPoller(const NetPoller &);
        NetPoller &operator =(const NetPoller &);
    public:
        NetPoller(Client *cli = NULL)
            : _cond(_mutex)
        {
            _client = cli;
            DLIST_INIT(&_avail_list);
            DLIST_INIT(&_list);
        }
        ~ NetPoller();

        void setClient(Client *cli) { _client = cli; }

        bool add(NetTalk *talk, int timeout);
        void cancel(NetTalk *talk);
        void cancelAll();
        int poll(NetTalk **talks, int count, int timeout_ms);

        void getTalks(std::vector<NetTalk *> &talks);
    private:
        int poll(NetTalk *talk);
        void done(NetStub *st);
        friend class NetProxy;
    private:
        Mutex _mutex;
        Cond _cond;
        __dlist_t _avail_list;
        __dlist_t _list;
        Client *_client;
};

#endif
