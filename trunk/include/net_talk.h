/*
 * =====================================================================================
 *
 *       Filename:  net_talk.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/29/2013 02:07:58 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *   Organization:  edu.whu
 *
 * =====================================================================================
 */
#ifndef __LS_CLIENT_NET_TALK_H__
#define __LS_CLIENT_NET_TALK_H__

#include "nethead.h"

enum
{
    NET_ST_INIT = 0,
    NET_ST_CANCELED,
    NET_ST_SEND_HEAD,
    NET_ST_SEND_BODY,
    NET_ST_RECV_HEAD,
    NET_ST_RECV_BODY,
    NET_ST_DONE,
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

struct NetTalk
{
    int _sock;
    int _status;
    int _errno;

    net_head_t _req_head;
    net_head_t _res_head;

    void *_req_buf;
    void *_res_buf;

    unsigned int _req_len;
    unsigned int _res_len;

    int _time;

    void *_inner_arg;
};

#endif
