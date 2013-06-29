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

struct NetTalk
{
    int _sock;
    int _status;
    int _errno;

    net_head_t _req_head;
    net_head_t _res_head;

    void *_req_buf;
    void *_res_buf;

    int _req_len;
    int _res_len;

    int _time;

    void *_inner_arg;
};

#endif
