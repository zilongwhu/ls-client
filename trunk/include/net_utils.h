/*
 * =====================================================================================
 *
 *       Filename:  net_utils.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/11/2014 10:58:52 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#ifndef __LS_CLIENT_NET_UTILS_H__
#define __LS_CLIENT_NET_UTILS_H__

#include <sys/socket.h>

int connect_ms(int sockfd, const struct sockaddr *addr, socklen_t socklen, int ms);

#endif
