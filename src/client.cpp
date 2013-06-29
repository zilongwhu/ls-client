/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2012年05月23日 15时24分57秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */
#include <string.h>
#include <limits.h>
#include "utils.h"
#include "client_epex.h"
#include "net_poller.h"

struct NetStub
{
    NetTalk *_talk;

    int _status; /*  */
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
    __dlist_t _list;

    NetStub(NetTalk *talk)
    {
        _talk = talk;
        _talk->_inner_arg = this;
        _status = 0;
        _timeout = -1;
        ::bzero(&_start_tm, sizeof _start_tm);
        ::bzero(&_done_tm, sizeof _done_tm);
        ::bzero(&_tm, sizeof _tm);
        DLIST_INIT(&_link);
        DLIST_INIT(&_list);
    }
};

void ClientEpex::attach(NetStub *st)
{
    AutoLock __lock(_mutex);
    DLIST_INSERT_B(&st->_link, &_attach_list);
}

void ClientEpex::detach(NetStub *st)
{
    AutoLock __lock(_mutex);
    if (!DLIST_EMPTY(&st->_link))
    {
        DLIST_REMOVE(&st->_link);
        return ;
    }
    DLIST_INSERT_B(&st->_link, &_detach_list);
}

void ClientEpex::run()
{

}

void NetPoller::add(NetTalk *tt)
{
    if (tt)
    {
        NetStub *st = new NetStub(tt);
        DLIST_INSERT_B(&st->_list, &_list);
        _epex->attach(st);
    }
}

void NetPoller::cancel(NetTalk *tt)
{
    if (tt)
    {
        NetStub *st = (NetStub *)tt->_inner_arg;
        if (st)
        {
            st->_status |= ~INT_MAX;
            _epex->detach(st);
        }
    }
}

void NetPoller::cancelAll()
{
    for (__dlist_t *p = DLIST_NEXT(&_list); p != &_list; p = DLIST_NEXT(p))
    {
        this->cancel(GET_OWNER(p, NetStub, _list)->_talk);
    }
}

void NetPoller::done(NetStub *st)
{
    if (st)
    {
        AutoLock __lock(_mutex);
        bool empty = DLIST_EMPTY(&_avail_list);
        DLIST_INSERT_B(&st->_link, &_avail_list);
        if (empty)
            _cond.signal();
    }
}

int NetPoller::poll(NetTalk **talks, int count, int timeout_ms)
{
    if (NULL == talks || count < 0)
        return -1;

    int i = 0;
    __dlist_t *ptr;
    NetStub *stub;
    struct timeval tv;
    struct timeval now;
    int eslap_tm;
    gettimeofday(&tv, NULL);

    AutoLock __lock(_mutex);
RETRY:
    for (; i < count && !DLIST_EMPTY(&_avail_list); ++i)
    {
        ptr = DLIST_NEXT(&_avail_list);
        stub = GET_OWNER(ptr, NetStub, _link);
        DLIST_REMOVE(ptr);
        talks[i] = stub->_talk;
        delete stub;
    }
    if (i >= count)
        return i;
    while (DLIST_EMPTY(&_avail_list))
    {
        gettimeofday(&now, NULL);
        eslap_tm = (now.tv_sec - tv.tv_sec) * 1000 + (now.tv_usec - tv.tv_usec)/1000;
        if (eslap_tm >= timeout_ms)
            return i;
        _cond.wait(timeout_ms - eslap_tm);
    }
    goto RETRY;
}
