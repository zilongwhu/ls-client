/*
 * =====================================================================================
 *
 *       Filename:  net_utils.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/11/2014 11:35:50 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include "log.h"
#include "error.h"
#include "net_utils.h"

int connect_ms(int sockfd, const struct sockaddr *addr, socklen_t socklen, int ms)
{
    if (sockfd < 0)
    {
        WARNING("invalid sockfd[%d]", sockfd);
        return -1;
    }
    if (NULL == addr)
    {
        WARNING("addr is NULL");
        return -1;
    }
    int ret = 0;
    int flags = fcntl(sockfd, F_GETFL);
    if (flags < 0)
    {
        WARNING("failed to call fcntl, error[%s]", strerror_t(errno));
        return -1;
    }
    if (0 == (flags & O_NONBLOCK))
    {
        if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            WARNING("failed to call fcntl, error[%s]", strerror_t(errno));
            return -1;
        }
    }
    if (connect(sockfd, addr, socklen) < 0)
    {
        if (errno != EINPROGRESS)
        {
            WARNING("connect failed, error[%s]", strerror_t(errno));
            ret = -1;
            goto DONE;
        }
        struct pollfd pfd;
        pfd.fd = sockfd;
        pfd.events = POLLIN | POLLOUT;
        pfd.revents = 0;
        ret = poll(&pfd, 1, ms);
        if (ret == 0)
        {
            WARNING("connect timeout[%d ms]", ms);
            ret = -2;
        }
        else if (ret == 1)
        {
            if ((pfd.revents & POLLIN) || (pfd.revents & POLLOUT))
            {
                int error = 0;
                socklen_t elen = sizeof(error);
                if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &elen) < 0)
                {
                    WARNING("getsockopt error[%s]", strerror_t(errno));
                    ret = -1;
                }
                else if (error)
                {
                    WARNING("socket error[%s]", strerror_t(error));
                    ret = -1;
                }
            }
        }
    }
DONE:
    if (0 == (flags & O_NONBLOCK))
    {
        if (fcntl(sockfd, F_SETFL, flags) < 0)
        {
            WARNING("failed to call fcntl, error[%s]", strerror_t(errno));
            return -1;
        }
    }
    return ret;
}
