/*
 * =====================================================================================
 *
 *       Filename:  server.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/11/2014 09:11:06 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#include "log.h"
#include "utils.h"
#include "error.h"
#include "server.h"
#include "net_utils.h"
#include <errno.h>
#include <arpa/inet.h>

Server::~Server()
{
    while (_socks.size() > 0)
    {
        int sock = _socks.front();
        SAFE_CLOSE(sock);
        _socks.pop_front();
    }
}

int Server::set_addr(const char *ptr, int port)
{
    if (NULL == ptr || port < 0)
    {
        WARNING("invalid args: ptr=%p, port=%d", ptr, port);
        return -1;
    }
    struct sockaddr_in *ai = (struct sockaddr_in *)&_addr;
    struct sockaddr_in6 *ai6 = (struct sockaddr_in6 *)&_addr;

    int ret = inet_pton(AF_INET, ptr, &ai->sin_addr.s_addr);
    if (ret == 1)
    {
        ai->sin_family = AF_INET;
        ai->sin_port = htons(port);
        _addrlen = sizeof(*ai);
        goto RET;
    }
    ret = inet_pton(AF_INET6, ptr, &ai6->sin6_addr.s6_addr);
    if (ret == 1)
    {
        ai->sin_family = AF_INET6;
        ai->sin_port = htons(port);
        _addrlen = sizeof(*ai6);
        goto RET;
    }
    WARNING("invalid address[%s][%d]", ptr, port);
    return -1;
RET:
    char buffer[256];
    snprintf(buffer, sizeof buffer, "%s : %d", ptr, port);
    _addrstr = buffer;
    return 0;
}

int Server::connect()
{
    int sock = socket(_addr.ss_family, SOCK_STREAM, 0);
    if (sock < 0)
    {
        WARNING("failed to create sock, error[%s]", strerror_t(errno));
    }
    else if (connect_ms(sock, (struct sockaddr *)&_addr, _addrlen, _connect_timeout) < 0)
    {
        WARNING("failed to connect to [%s]", _addrstr.c_str());

        SAFE_CLOSE(sock);
        sock = -1;
    }
    _healthy = (sock >= 0);
    return sock;
}

int Server::get_avail_sock()
{
    int sock = -1;
    if (_pool_size > 0)
    {
        AutoLock __lock(_mutex);
        while (_socks.size() > 0)
        {
            sock = _socks.front();
            _socks.pop_front();

            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            {
                WARNING("getsockopt error[%s]", strerror_t(errno));
            }
            else if (error)
            {
                WARNING("getsockopt error[%s]", strerror_t(error));
            }
            else break;
            SAFE_CLOSE(sock);
            sock = -1;
        }
    }
    if (sock < 0)
    {
        sock = this->connect();
    }
    return sock;
}

void Server::return_sock(int sock, bool is_ok)
{
    if (sock < 0)
    {
        return ;
    }
    AutoLock __lock(_mutex);
    if (_pool_size <= 0 || int(_socks.size()) >= _pool_size)
    {
        SAFE_CLOSE(sock);
    }
    else
    {
        _socks.push_back(sock);
    }
    if (_stats_off >= sizeof(_stats)/sizeof(_stats[0]))
    {
        _stats_off = 0;
    }
    if (is_ok)
    {
        if (_stats[_stats_off])
        {
            _stats[_stats_off] = 0;
            --_fail_count;
        }
    }
    else
    {
        if (!_stats[_stats_off])
        {
            _stats[_stats_off] = 1;
            ++_fail_count;
        }
    }
    ++_stats_off;
}
