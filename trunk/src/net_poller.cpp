/*
 * =====================================================================================
 *
 *       Filename:  net_poller.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年06月30日 11时57分25秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */
#include "utils.h"
#include "net_poller.h"
#include "client_epex.h"

NetPoller::~NetPoller()
{
    while (!DLIST_EMPTY(&_list))
    {
        NetTalk *talk;
        this->poll(&talk, 1, -1);
    }
    DLIST_INIT(&_avail_list);
    DLIST_INIT(&_list);
    _epex = NULL;
}

bool NetPoller::add(NetTalk *talk)
{
    if (talk)
    {
        NetStub *st = new NetStub(talk, this);
        if (st)
        {
            DLIST_INSERT_B(&st->_list, &_list);
            _epex->attach(st);
            return true;
        }
    }
    return false;
}

void NetPoller::cancel(NetTalk *talk)
{
    if (talk)
    {
        NetStub *st = (NetStub *)talk->_inner_arg;
        if (st)
        {
            _epex->detach(st);
            this->poll(talk);
        }
    }
}

void NetPoller::cancelAll()
{
    __dlist *p;
    while (!DLIST_EMPTY(&_list))
    {
        p = DLIST_NEXT(&_list);
        this->cancel(GET_OWNER(p, NetStub, _list)->_talk);
    }
}

void NetPoller::done(NetStub *st)
{
    if (st)
    {
        AutoLock __lock(_mutex);
        DLIST_INSERT_B(&st->_avail_list, &_avail_list);
        _cond.signal();
    }
}

int NetPoller::poll(NetTalk *talk)
{
    if (NULL == talk)
        return -1;
    __dlist_t *ptr;
    NetStub *st;
    AutoLock __lock(_mutex);
RETRY:
    for (ptr = DLIST_NEXT(&_avail_list);
            ptr != &_avail_list; ptr = DLIST_NEXT(ptr))
    {
        st = GET_OWNER(ptr, NetStub, _avail_list);
        if (st->_talk == talk)
        {
            delete st; /* free and remove from lists */
            return 0;
        }
    }
    _cond.wait(-1);
    goto RETRY;
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
    while (i < count && !DLIST_EMPTY(&_avail_list))
    {
        ptr = DLIST_NEXT(&_avail_list);
        stub = GET_OWNER(ptr, NetStub, _avail_list);
        if (!stub->_cancel) /* not canceled by user */
            talks[i++] = stub->_talk;
        delete stub;/* free and remove from lists */
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
