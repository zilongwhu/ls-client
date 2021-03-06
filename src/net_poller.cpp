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
#include <new>
#include "utils.h"
#include "net_poller.h"
#include "client.h"

NetPoller::~NetPoller()
{
    while (!DLIST_EMPTY(&_list))
    {
        NetTalk *talk;
        this->poll(&talk, 1, -1);
    }
    DLIST_INIT(&_avail_list);
    DLIST_INIT(&_list);
    _client = NULL;
}

bool NetPoller::add(NetTalk *talk, int timeout)
{
    if (talk)
    {
        NetStub *st = new(std::nothrow) NetStub(talk, this);
        if (st)
        {
            st->_timeout = timeout;
            DLIST_INSERT_B(&st->_list, &_list);
            _client->attach(st);
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
            _client->detach(st);
            this->poll(talk);
            talk->_status = NET_ST_CANCELED;
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
        st->_done = 1;
        DLIST_INSERT_B(&st->_avail_list, &_avail_list);
        _cond.signal();
    }
}

int NetPoller::poll(NetTalk *talk)
{
    if (NULL == talk)
        return -1;
    NetStub *st;
    AutoLock __lock(_mutex);
RETRY:
    st = (NetStub *)talk->_inner_arg;
    if (st->_done)
    {
        delete st;
        return 0;
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
        if (timeout_ms < 0)
        {
            _cond.wait(-1);
            continue;
        }
        gettimeofday(&now, NULL);
        eslap_tm = (now.tv_sec - tv.tv_sec) * 1000 + (now.tv_usec - tv.tv_usec)/1000;
        if (eslap_tm >= timeout_ms)
            return i;
        _cond.wait(timeout_ms - eslap_tm);
    }
    goto RETRY;
}

void NetPoller::getTalks(std::vector<NetTalk *> &talks)
{
    talks.clear();

    __dlist *p;
    while (!DLIST_EMPTY(&_list))
    {
        p = DLIST_NEXT(&_list);
        talks.push_back(GET_OWNER(p, NetStub, _list)->_talk);
    }
}
