/*
 * =====================================================================================
 *
 *       Filename:  net_stub.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年06月29日 19时34分09秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *   Organization:  edu.whu
 *
 * =====================================================================================
 */

#ifndef __LS_CLIENT_NET_STUB_H__
#define __LS_CLIENT_NET_STUB_H__

#include <string.h>
#include <sys/time.h>
#include "dlist.h"
#include "net_talk.h"

enum
{
    NETSTUB_INIT = 0,
    NETSTUB_SEND_HEAD,
    NETSTUB_SEND_BODY,
    NETSTUB_RECV_HEAD,
    NETSTUB_RECV_BODY,
    NETSTUB_DONE,
};

enum
{
    NET_ERR_ATTACH_FAIL = -1,
    NET_ERR_TIMEOUT = -2,
    NET_ERR_CLOSED = -3,
    NET_ERR_READ = -4,
    NET_ERR_WRITE = -5,
    NET_ERR_MAGIC_NUM = -6,
    NET_ERR_BIG_RESP = -7,
};

class NetPoller;
struct NetStub
{
    NetTalk *_talk;
    NetPoller *_poller;

    int _status;
    int _errno;
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
    __dlist_t _att_list;

    NetStub(NetTalk *talk, NetPoller *poller)
    {
        _talk = talk;
        _talk->_inner_arg = this;
        _poller = poller;
        _status = NETSTUB_INIT;
        _timeout = -1;
        ::bzero(&_start_tm, sizeof _start_tm);
        ::bzero(&_done_tm, sizeof _done_tm);
        ::bzero(&_tm, sizeof _tm);
        DLIST_INIT(&_link);
        DLIST_INIT(&_list);
        DLIST_INIT(&_att_list);
    }

    ~NetStub()
    {
        if (_talk)
            _talk->_inner_arg = NULL;
        _talk = NULL;
        _poller = NULL;
        DLIST_REMOVE(&_link);
        DLIST_REMOVE(&_list);
        DLIST_REMOVE(&_att_list);
    }
};

#endif