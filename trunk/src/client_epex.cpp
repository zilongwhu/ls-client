/*
 * =====================================================================================
 *
 *       Filename:  client_epex.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年06月30日 11时56分18秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */
#include <limits.h>
#include "log.h"
#include "utils.h"
#include "client_epex.h"
#include "net_poller.h"

void ClientEpex::attach(NetStub *st)
{
    gettimeofday(&st->_start_tm, NULL);
    AutoLock __lock(_mutex);
    DLIST_INSERT_B(&st->_att_list, &_attach_list);
}

void ClientEpex::detach(NetStub *st)
{
    st->_status |= ~INT_MAX;
    {
        AutoLock __lock(_mutex);
        if (!DLIST_EMPTY(&st->_att_list))
        {
            DLIST_REMOVE(&st->_att_list);/* still in _attach_list */
        }
        else return ;
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
#define SET_ERROR(errno)                                   \
    do                                                     \
    {                                                      \
        st->_errno = errno;                                \
        epex_detach(_epex, st->_talk->_sock, NULL);        \
    } while(0)

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

    _stop = false;
    while (!_stop)
    {
        DLIST_INIT(&attach_ok_list);
        DLIST_INIT(&attach_fail_list);
        {
            AutoLock __lock(_mutex);
            while (!DLIST_EMPTY(&_attach_list))
            {
                ptr = DLIST_NEXT(&_attach_list);
                st = GET_OWNER(ptr, NetStub, _att_list);
                DLIST_REMOVE(ptr);
                if (epex_attach(_epex, st->_talk->_sock, st, -1))
                    DLIST_INSERT_B(&st->_tmp_list, &attach_ok_list);
                else
                    DLIST_INSERT_B(&st->_tmp_list, &attach_fail_list);
            }
        }
        gettimeofday(&now, NULL);
        while (!DLIST_EMPTY(&attach_fail_list))
        {
            ptr = DLIST_NEXT(&attach_fail_list);
            st = GET_OWNER(ptr, NetStub, _tmp_list);
            DLIST_REMOVE(ptr);
            st->_errno = NET_ERR_ATTACH_FAIL;
            this->done(st, &now);
        }
        while (!DLIST_EMPTY(&attach_ok_list))
        {
            ptr = DLIST_NEXT(&attach_ok_list);
            st = GET_OWNER(ptr, NetStub, _tmp_list);
            DLIST_REMOVE(ptr);
            st->_talk->_req_head._magic_num = MAGIC_NUM;
            st->_talk->_req_head._body_len = st->_talk->_req_len;
            st->_status = NETSTUB_SEND_HEAD;
            if (epex_write(_epex, st->_talk->_sock, &st->_talk->_req_head,
                        sizeof(st->_talk->_req_head), NULL, st->_timeout))
            {
                TRACE("try to send head to sock[%d]", st->_talk->_sock);
            }
            else
            {
                SET_ERROR(NET_ERR_WRITE);
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
                    SET_ERROR(NET_ERR_CLOSED);
                    continue;
                case NET_ETIMEOUT:
                    SET_ERROR(NET_ERR_TIMEOUT);
                    continue;
                case NET_ERROR:
                    st->_errno = res._errno;
                    this->done(st, &now);
                    continue;
                case NET_EDETACHED:
                    this->done(st, &now);
                    continue;
                default:
                    WARNING("should not be here, res._status=%hd", res._status);
                    continue;
            }
            sock = st->_talk->_sock;
            if (st->_status < 0) /* canceled by user */
            {
                TRACE("sock[%d] is canceled by user", sock);
                epex_detach(_epex, sock, NULL);
                continue;
            }
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
                        SET_ERROR(NET_ERR_TIMEOUT);
                        continue;
                    }
                    if (!epex_write(_epex, sock, st->_talk->_req_buf, st->_talk->_req_len,
                                NULL, tm_left))
                    {
                        SET_ERROR(NET_ERR_WRITE);
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
                        SET_ERROR(NET_ERR_TIMEOUT);
                        continue;
                    }
                    if (!epex_read(_epex, sock, &st->_talk->_res_head, sizeof(st->_talk->_res_head),
                                NULL, tm_left))
                    {
                        SET_ERROR(NET_ERR_READ);
                        continue;
                    }
                    TRACE("try to recv head from sock[%d]", sock);
                    break;
                case NETSTUB_RECV_HEAD:
                    TRACE("recv head from sock[%d] ok", sock);
                    st->_tm._recv_head = elasp_tm - st->_tm._send_body - st->_tm._send_head;
                    if (st->_talk->_res_head._magic_num != MAGIC_NUM)
                    {
                        SET_ERROR(NET_ERR_MAGIC_NUM);
                        continue;
                    }
                    if (st->_talk->_res_head._body_len > st->_talk->_res_len)
                    {
                        SET_ERROR(NET_ERR_BIG_RESP);
                        continue;
                    }
                    st->_status = NETSTUB_RECV_BODY;
                    if (tm_left == 0)
                    {
                        SET_ERROR(NET_ERR_TIMEOUT);
                        continue;
                    }
                    if (!epex_read(_epex, sock, st->_talk->_res_buf, st->_talk->_res_head._body_len,
                                NULL, tm_left))
                    {
                        SET_ERROR(NET_ERR_READ);
                        continue;
                    }
                    TRACE("try to recv body[%u] from sock[%d]", st->_talk->_res_head._body_len, sock);
                    break;
                case NETSTUB_RECV_BODY:
                    TRACE("recv body[%u] from sock[%d] ok", st->_talk->_res_head._body_len, sock);
                    st->_tm._recv_body = elasp_tm - st->_tm._recv_head
                        - st->_tm._send_body - st->_tm._send_head;
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
    NOTICE("NetPoller is stoped now");
#undef SET_ERROR
}

