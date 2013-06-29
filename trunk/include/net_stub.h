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

    ~NetStub()
    {
        if (_talk)
            _talk->_inner_arg = NULL;
        _talk = NULL;
        DLIST_REMOVE(&_link);
        DLIST_REMOVE(&_list);
    }
};

#endif
