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

class NetPoller;
struct NetStub
{
    NetTalk *_talk;
    NetPoller *_poller;

    int8_t _idx;
    int8_t _done;
    int8_t _cancel;
    int8_t _status;
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

    __dlist_t _list;
    __dlist_t _avail_list;
    __dlist_t _att_list;
    __dlist_t _tmp_list;

    NetStub(NetTalk *talk, NetPoller *poller)
    {
        _talk = talk;
        _talk->_inner_arg = this;
        _poller = poller;
        _idx = 0;
        _done = 0;
        _cancel = 0;
        _status = NET_ST_INIT;
        _errno = 0;
        _timeout = -1;
        ::bzero(&_start_tm, sizeof _start_tm);
        ::bzero(&_done_tm, sizeof _done_tm);
        ::bzero(&_tm, sizeof _tm);
        DLIST_INIT(&_list);
        DLIST_INIT(&_avail_list);
        DLIST_INIT(&_att_list);
        DLIST_INIT(&_tmp_list);
    }

    ~NetStub()
    {
        if (_talk)
            _talk->_inner_arg = NULL;
        _talk = NULL;
        _poller = NULL;
        DLIST_REMOVE(&_list);
        DLIST_REMOVE(&_avail_list);
        DLIST_REMOVE(&_att_list);
        DLIST_REMOVE(&_tmp_list);
    }
};

#endif
