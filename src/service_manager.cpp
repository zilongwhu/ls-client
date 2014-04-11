/*
 * =====================================================================================
 *
 *       Filename:  service_manager.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/11/2014 11:35:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#include "log.h"
#include "configure.h"
#include "service_manager.h"

void ServiceManager::clear()
{
    std::map<std::string, Service *>::iterator it = _service_map.begin();
    while (it != _service_map.end())
    {
        delete it->second;
        ++it;
    }
    _service_map.clear();
}

int ServiceManager::init(const char *path, const char *file)
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

    int service_num;
    int server_num;
    server_args args;
    std::string service_name;

    if (!conf.get("service_num", service_num))
    {
        WARNING("must have service_num");
        return -1;
    }
    if (service_num <= 0)
    {
        WARNING("service_num is %d, less than 0", service_num);
        return -1;
    }
    for (int i = 0; i < service_num; ++i)
    {
        char buffer[1024];

        snprintf(buffer, sizeof buffer, "service_%d_name", i);
        if (!conf.get(buffer, service_name))
        {
            goto FAIL;
        }
        WARNING("%s: %s", buffer, service_name.c_str());

        if (_service_map.find(service_name) != _service_map.end())
        {
            WARNING("ignore duplicate service[%s]", service_name.c_str());
            continue;
        }

        snprintf(buffer, sizeof buffer, "service_%d_num", i);
        if (!conf.get(buffer, server_num))
        {
            goto FAIL;
        }
        WARNING("%s: %d", buffer, server_num);

        if (server_num <= 0)
        {
            WARNING("ignore empty service[%s]", service_name.c_str());
            continue;
        }

        std::vector<server_args> service_args;
        for (int j = 0; j < server_num; ++j)
        {
            snprintf(buffer, sizeof buffer, "%s_%d_ip", service_name.c_str(), j);
            if (!conf.get(buffer, args.addr))
            {
                goto FAIL;
            }
            WARNING("%s: %s", buffer, args.addr.c_str());

            snprintf(buffer, sizeof buffer, "%s_%d_port", service_name.c_str(), j);
            if (!conf.get(buffer, args.port))
            {
                goto FAIL;
            }
            WARNING("%s: %d", buffer, args.port);

            snprintf(buffer, sizeof buffer, "%s_%d_pool_size", service_name.c_str(), j);
            if (!conf.get(buffer, args.pool_size))
            {
                goto FAIL;
            }
            WARNING("%s: %d", buffer, args.pool_size);

            snprintf(buffer, sizeof buffer, "%s_%d_connect_timeout", service_name.c_str(), j);
            if (!conf.get(buffer, args.connect_timeout))
            {
                goto FAIL;
            }
            WARNING("%s: %d", buffer, args.connect_timeout);

            service_args.push_back(args);
        }
        Service *svc = new (std::nothrow) Service;
        if (NULL == svc)
        {
            WARNING("failed to new Service, i=%d", i);
            goto FAIL;
        }
        if (svc->init(service_args) < 0)
        {
            WARNING("failed to init Service, i=%d", i);
            delete svc;
            goto FAIL;
        }
        _service_map.insert(std::make_pair(service_name, svc));
    }
    WARNING("init service manager ok");
    return 0;
FAIL:
    this->clear();
    return -1;
}
