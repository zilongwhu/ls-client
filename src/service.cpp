/*
 * =====================================================================================
 *
 *       Filename:  service.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
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
#include <stdio.h>

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
    std::string name;
    server_args args;
    std::vector<server_args> service_args;

    if (!conf.get("server_name", name))
    {
        WARNING("must have server_name");
        return -1;
    }
    WARNING("service_name: %s", name.c_str());

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
    WARNING("server_num: %d", num);

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
    return this->init(name, service_args);
}

int Service::init(const std::string &name, const std::vector<server_args> &args)
{
    _servers = new Server[args.size()];
    if (NULL == _servers)
    {
        WARNING("service[%s]: failed to new Server, num=%d", name.c_str(), int(args.size()));
        return -1;
    }
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (_servers[i].set_addr(args[i].addr.c_str(), args[i].port) < 0)
        {
            WARNING("service[%s]: failed to init server %d", name.c_str(), i);
            goto FAIL;
        }
        _servers[i].set_pool_size(args[i].pool_size);
        _servers[i].set_connect_timeout(args[i].connect_timeout);
    }
    _server_num = args.size();
    _name = name;
    WARNING("init Service[%s] ok", _name.c_str());
    return 0;
FAIL:
    delete [] _servers;
    _servers = NULL;
    _server_num = 0;
    return -1;
}

int Service::get_connection(Connection &conn)
{
    if (conn._key < 0)
    {
        conn._key = rand();
    }
    int avail_server_num = 0;
    std::vector<bool> flags(_server_num, true);
    for (int i = 0; i < _server_num; ++i)
    {
        flags[i] = _servers[i].is_healthy();
        if (flags[i])
        {
            ++avail_server_num;
        }
    }
    TRACE("service[%s]: avail_server_num=%d, server_num=%d", _name.c_str(), avail_server_num, _server_num);
    for (int n = 0; n < 3 && avail_server_num > 0; ++n) /* retry */
    {
        int idx = conn._key % avail_server_num;
        for (int i = 0; i < _server_num; ++i)
        {
            if (flags[i])
            {
                if (idx == 0)
                {
                    idx = i;
                    break;
                }
                --idx;
            }
        }
        int suc_ratio = 100 - _servers[idx].fail_ratio();
        if (suc_ratio < 10)
        {
            suc_ratio = 10;
        }
        TRACE("service[%s]: try to use server, idx=%d, success ratio=%d, try time=%d", _name.c_str(), idx, suc_ratio, n);
        if (rand() % 100 > suc_ratio)
        {
            flags[idx] = false;
            --avail_server_num;
            continue;
        }
        TRACE("service[%s]: use server, idx=%d", _name.c_str(), idx);
        conn._sock = _servers[idx].get_avail_sock();
        if (conn._sock >= 0)
        {
            conn._server = &_servers[idx];
            TRACE("service[%s]: get connection ok from server, idx=%d", _name.c_str(), idx);
            return 0;
        }
        else
        {
            flags[idx] = false;
            --avail_server_num;
            WARNING("service[%s]: server becomes unhealthy, idx=%d", _name.c_str(), idx);
        }
    }
    int ret = -1;
    int last_idx = rand() % _server_num;
    for (int i = 0; i < _server_num; ++i)
    {
        int idx = (last_idx + i) % _server_num;
        if (_servers[idx].is_healthy())
        {
            TRACE("service[%s]: last chance, use server, idx=%d", _name.c_str(), idx);
            conn._sock = _servers[idx].get_avail_sock();
            if (conn._sock >= 0)
            {
                ret = 0;
                conn._server = &_servers[idx];
                TRACE("service[%s]: get connection ok from server, idx=%d", _name.c_str(), idx);
            }
            break;
        }
    }
    return ret;
}

void Service::check_healthy()
{
    for (int i = 0; i < _server_num; ++i)
    {
        if (!_servers[i].is_healthy())
        {
            int sock = _servers[i].connect();
            if (sock >= 0)
            {
                _servers[i].return_sock(sock, true);
                WARNING("service[%s]: server becomes healthy again, idx=%d", _name.c_str(), i);
            }
        }
    }
}
