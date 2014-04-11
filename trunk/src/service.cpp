/*
 * =====================================================================================
 *
 *       Filename:  server.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/11/2014 09:11:06 PM
 *        Created:  04/11/2014 11:12:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#include "log.h"
#include "service.h"
#include "configure.h"

int Service::init(const char *path, const char *file)
{
    if (NULL == path || NULL == file)
    {
        WARNING("invalid args: path=%p, file=%p", path, file);
        return -1;
    }
    Config conf(path, file);
    if (conf.parse() < 0)
    {
        WARNING("failed to parse conf file[%s][%s]", path, file);
        return -1;
    }

    int num;
    int port;
    int pool_size;
    int connect_timeout;
    std::string addr;

    if (!conf.get("server_num", num))
    {
        WARNING("must have server_num");
        return -1;
    }
    if (num <= 0)
    {
        WARNING("server_num is %d, less than 0", num);
        return -1;
    }
    _servers = new Server[num];
    if (NULL == _servers)
    {
        WARNING("failed to new servers, num=%d", num);
        return -1;
    }
    for (int i = 0; i < num; ++i)
    {
        char buffer[256];

        snprintf(buffer, sizeof buffer, "server_%d_ip", i);
        if (!conf.get(buffer, addr))
        {
            goto FAIL;
        }
        WARNING("%s: %s", buffer, addr.c_str());

        snprintf(buffer, sizeof buffer, "server_%d_port", i);
        if (!conf.get(buffer, port))
        {
            goto FAIL;
        }
        WARNING("%s: %d", buffer, port);

        snprintf(buffer, sizeof buffer, "server_%d_pool_size", i);
        if (!conf.get(buffer, pool_size))
        {
            goto FAIL;
        }
        WARNING("%s: %d", buffer, pool_size);

        snprintf(buffer, sizeof buffer, "server_%d_connect_timeout", i);
        if (!conf.get(buffer, connect_timeout))
        {
            goto FAIL;
        }
        WARNING("%s: %d", buffer, connect_timeout);

        if (_servers[i].set_addr(addr.c_str(), port) < 0)
        {
            WARNING("failed to init server %d", i);
            goto FAIL;
        }
        _servers[i].set_pool_size(pool_size);
        _servers[i].set_connect_timeout(connect_timeout);
    }
    _server_num = num;
    WARNING("init service ok");
    return 0;
FAIL:
    delete [] _servers;
    _servers = NULL;
    return -1;
}

int Service::init(const std::vector<server_args> &args)
{
    return 0;
}
