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
    server_args args;
    std::vector<server_args> service_args;

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
    for (int i = 0; i < num; ++i)
    {
        char buffer[256];

        snprintf(buffer, sizeof buffer, "server_%d_ip", i);
        if (!conf.get(buffer, args.addr))
        {
            WARNING("cannot get %s", buffer);
            return -1;
        }
        WARNING("%s: %s", buffer, args.addr.c_str());

        snprintf(buffer, sizeof buffer, "server_%d_port", i);
        if (!conf.get(buffer, args.port))
        {
            WARNING("cannot get %s", buffer);
            return -1;
        }
        WARNING("%s: %d", buffer, args.port);

        snprintf(buffer, sizeof buffer, "server_%d_pool_size", i);
        if (!conf.get(buffer, args.pool_size))
        {
            WARNING("cannot get %s", buffer);
            return -1;
        }
        WARNING("%s: %d", buffer, args.pool_size);

        snprintf(buffer, sizeof buffer, "server_%d_connect_timeout", i);
        if (!conf.get(buffer, args.connect_timeout))
        {
            WARNING("cannot get %s", buffer);
            return -1;
        }
        WARNING("%s: %d", buffer, args.connect_timeout);

        service_args.push_back(args);
    }
    return this->init(service_args);
}

int Service::init(const std::vector<server_args> &args)
{
    _servers = new Server[args.size()];
    if (NULL == _servers)
    {
        WARNING("failed to new Server, num=%d", int(args.size()));
        return -1;
    }
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (_servers[i].set_addr(args[i].addr.c_str(), args[i].port) < 0)
        {
            WARNING("failed to init server %d", i);
            goto FAIL;
        }
        _servers[i].set_pool_size(args[i].pool_size);
        _servers[i].set_connect_timeout(args[i].connect_timeout);
    }
    _server_num = args.size();
    WARNING("init Service ok");
    return 0;
FAIL:
    delete [] _servers;
    _servers = NULL;
    _server_num = 0;
    return -1;
}
