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
#include "log.h"
#include "utils.h"
#include "client_epex.h"
#include "net_poller.h"

void ClientEpex::attach(NetStub *st)
{
    gettimeofday(&st->_start_tm, NULL);
    AutoLock __lock(_mutex);
    DLIST_INSERT_B(&st->_link, &_attach_list);
}

void ClientEpex::detach(NetStub *st)
{
    st->_status |= ~INT_MAX;
    {
        AutoLock __lock(_mutex);
        if (!DLIST_EMPTY(&st->_link))
        {
            DLIST_REMOVE(&st->_link);
        }
        else
        {
            DLIST_INSERT_B(&st->_link, &_detach_list);
            return ;
        }
    }
    this->done(st);
}

void ClientEpex::done(NetStub *st, struct timeval *now)
{
    if (now)
        st->_done_tm = *now;
    else
        gettimeofday(&st->_done_tm, NULL);
    st->_tm._total = (st->_done_tm.tv_sec - st->_start_tm.tv_sec) * 1000
        + (st->_done_tm.tv_usec - st->_start_tm.tv_usec) / 1000;
    st->_poller->done(st);
}

void ClientEpex::run()
{
    ssize_t ret;
    netresult_t results[20];
    __dlist_t attach_ok_list;
    __dlist_t attach_fail_list;
    __dlist_t *ptr;
    NetStub *st;
    int sock;
    int tm_left;
    int elasp_tm;
    struct timeval now;
    while (!_stop)
    {
        DLIST_INIT(&attach_ok_list);
        DLIST_INIT(&attach_fail_list);
        {
            AutoLock __lock(_mutex);
            while (!DLIST_EMPTY(&_detach_list))
            {
                ptr = DLIST_NEXT(&_detach_list);
                st = GET_OWNER(ptr, NetStub, _link);
                DLIST_REMOVE(ptr);
                epex_detach(_epex, st->_talk->_sock, NULL);
            }
            while (!DLIST_EMPTY(&_attach_list))
            {
                ptr = DLIST_NEXT(&_attach_list);
                st = GET_OWNER(ptr, NetStub, _link);
                DLIST_REMOVE(ptr);
                if (epex_attach(_epex, st->_talk->_sock, st, -1))
                    DLIST_INSERT_B(&st->_att_list, &attach_ok_list);
                else
                    DLIST_INSERT_B(&st->_att_list, &attach_fail_list);
            }
        }
        gettimeofday(&now, NULL);
        while (!DLIST_EMPTY(&attach_fail_list))
        {
            ptr = DLIST_NEXT(&attach_fail_list);
            st = GET_OWNER(ptr, NetStub, _att_list);
            DLIST_REMOVE(ptr);
            st->_errno = NET_ERR_ATTACH_FAIL;
            this->done(st, &now);
        }
        while (!DLIST_EMPTY(&attach_ok_list))
        {
            ptr = DLIST_NEXT(&attach_ok_list);
            st = GET_OWNER(ptr, NetStub, _att_list);
            DLIST_REMOVE(ptr);
            st->_talk->_req_head._magic_num = MAGIC_NUM;
            st->_talk->_req_head._body_len = st->_talk->_req_len;
            st->_status = NETSTUB_SEND_HEAD;
            if (!epex_write(_epex, st->_talk->_sock, &st->_talk->_req_head,
                        sizeof(st->_talk->_req_head), NULL, st->_timeout))
            {
                st->_errno = NET_ERR_WRITE;
                epex_detach(_epex, st->_talk->_sock, NULL);
            }
        }
        ret = epex_poll(_epex, results, sizeof(results)/sizeof(results[0]));
        gettimeofday(&now, NULL);
        for (long i = 0; i < ret; ++i)
        {
            const netresult_t &res = results[i];
            st = (NetStub *)res._user_ptr2;
            switch (res._status)
            {
                case NET_DONE:
                    break;
                case NET_ECLOSED:
                    st->_errno = NET_ERR_CLOSED;
                    epex_detach(_epex, sock, NULL);
                    continue;
                case NET_ETIMEOUT:
                    st->_errno = NET_ERR_TIMEOUT;
                    epex_detach(_epex, sock, NULL);
                    continue;
                case NET_ERROR:
                    st->_errno = res._errno;
                    this->done(st, &now);
                    continue;
                case NET_EDETACHED:
                    this->done(st, &now);
                    continue;
                default:
                    WARNING("should not be here");
                    continue;
            }
            sock = st->_talk->_sock;
            elasp_tm = (now.tv_sec - st->_start_tm.tv_sec) * 1000
                + (now.tv_usec - st->_start_tm.tv_usec) / 1000;
            if (st->_timeout < 0)
                tm_left = -1;
            else if (st->_timeout > elasp_tm)
                tm_left = st->_timeout - elasp_tm;
            else
                tm_left = 0;
            switch (st->_status)
            {
                case NETSTUB_SEND_HEAD:
                    TRACE("send head to sock[%d] ok", sock);
                    st->_tm._send_head = elasp_tm;
                    st->_status = NETSTUB_SEND_BODY;
                    if (tm_left == 0)
                    {
                        st->_errno = NET_ERR_TIMEOUT;
                        epex_detach(_epex, sock, NULL);
                        continue;
                    }
                    if (!epex_write(_epex, sock, st->_talk->_req_buf, st->_talk->_req_len,
                                NULL, tm_left))
                    {
                        st->_errno = NET_ERR_WRITE;
                        epex_detach(_epex, sock, NULL);
                        continue;
                    }
                    TRACE("try to send body[%u] to sock[%d]", st->_talk->_req_len, sock);
                    break;
                case NETSTUB_SEND_BODY:
                    TRACE("send body[%u] to sock[%d] ok", st->_talk->_req_len, sock);
                    st->_tm._send_body = elasp_tm - st->_tm._send_head;
                    st->_status = NETSTUB_RECV_HEAD;
                    if (tm_left == 0)
                    {
                        st->_errno = NET_ERR_TIMEOUT;
                        epex_detach(_epex, sock, NULL);
                        continue;
                    }
                    if (!epex_read(_epex, sock, &st->_talk->_res_head, sizeof(st->_talk->_res_head),
                                NULL, tm_left))
                    {
                        st->_errno = NET_ERR_WRITE;
                        epex_detach(_epex, sock, NULL);
                        continue;
                    }
                    TRACE("try to recv head from sock[%d]", sock);
                    break;
                case NETSTUB_RECV_HEAD:
                    TRACE("recv head from sock[%d] ok", sock);
                    st->_tm._recv_head = elasp_tm - st->_tm._send_body - st->_tm._send_head;
                    if (st->_talk->_res_head._magic_num != MAGIC_NUM)
                    {
                        st->_errno = NET_ERR_MAGIC_NUM;
                        epex_detach(_epex, sock, NULL);
                        continue;
                    }
                    if (st->_talk->_res_head._body_len > st->_talk->_res_len)
                    {
                        st->_errno = NET_ERR_BIG_RESP;
                        epex_detach(_epex, sock, NULL);
                        continue;
                    }
                    st->_status = NETSTUB_RECV_BODY;
                    if (tm_left == 0)
                    {
                        st->_errno = NET_ERR_TIMEOUT;
                        epex_detach(_epex, sock, NULL);
                        continue;
                    }
                    if (!epex_read(_epex, sock, st->_talk->_res_buf, st->_talk->_res_head._body_len,
                                NULL, tm_left))
                    {
                        st->_errno = NET_ERR_WRITE;
                        epex_detach(_epex, sock, NULL);
                        continue;
                    }
                    TRACE("try to recv body[%u] from sock[%d]", st->_talk->_res_head._body_len, sock);
                    break;
                case NETSTUB_RECV_BODY:
                    TRACE("recv body[%u] from sock[%d] ok", st->_talk->_res_head._body_len, sock);
                    st->_tm._recv_body = elasp_tm - st->_tm._recv_head - st->_tm._send_body - st->_tm._send_head;
                    st->_status = NETSTUB_DONE;
                    epex_detach(_epex, sock, NULL);
                    TRACE("talk with sock[%d] ok", sock);
                    break;
                default:
                    WARNING("should not be here");
                    break;
            }
        }
    }
}

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

void NetPoller::add(NetTalk *talk)
{
    if (talk)
    {
        NetStub *st = new NetStub(talk, this);
        DLIST_INSERT_B(&st->_list, &_list);
        _epex->attach(st);
    }
}

void NetPoller::cancel(NetTalk *talk)
{
    if (talk)
    {
        NetStub *st = (NetStub *)talk->_inner_arg;
        if (st)
        {
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
    while (i < count && !DLIST_EMPTY(&_avail_list))
    {
        ptr = DLIST_NEXT(&_avail_list);
        stub = GET_OWNER(ptr, NetStub, _link);
        if (stub->_status >= 0) /* not canceled by user */
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
