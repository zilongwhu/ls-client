/*
 * =====================================================================================
 *
 *       Filename:  client_manager.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/12/2014 01:38:32 PM
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
#include "client_manager.h"

int ClientManager::init(const char *path, const char *file)
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
    int proxy_num = 1;
    if (conf.get("proxy_num", proxy_num) && proxy_num <= 0)
    {
        WARNING("invalid proxy_num");
        return -1;
    }
    WARNING("proxy_num=%d", proxy_num);
    if (_client.init(proxy_num) < 0)
    {
        WARNING("failed to init client");
        return -1;
    }
    WARNING("init client ok");
    if (_services.init(path, file) < 0)
    {
        WARNING("failed to init services");
        return -1;
    }
    WARNING("init services ok");
    return 0;
}

int ClientManager::post_request(const char *service_name, NetTalkWithConn *talk, int timeout, NetPoller *poller)
{
    if (NULL == service_name || NULL == talk || NULL == poller)
    {
        WARNING("invalid args");
        return -1;
    }
    if (_services.get_connection(service_name, talk->_conn) < 0)
    {
        WARNING("failed to get connection from service[%s]", service_name);
        return -1;
    }
    if (!poller->add(talk, timeout))
    {
        _services.return_connection(talk->_conn, true);
        WARNING("failed to add to poller");
        return -1;
    }
    return 0;
}

int ClientManager::poll(NetTalkWithConn **talks, int count, int timeout_ms, NetPoller *poller)
{
    int ret = poller->poll((NetTalk **)talks, count, timeout_ms);
    for (int i = 0; i < ret; ++i)
    {
        _services.return_connection(talks[i]->_conn, talks[i]->_status == NET_ST_DONE);
    }
    return ret;
}

void ClientManager::cancel(NetTalkWithConn *talk, NetPoller *poller)
{
    poller->cancel(talk);
    _services.return_connection(talk->_conn, false);
}

void ClientManager::cancelAll(NetPoller *poller)
{
    std::vector<NetTalk *> talks;
    poller->getTalks(talks);
    poller->cancelAll();
    for (size_t i = 0; i < talks.size(); ++i)
    {
        _services.return_connection(((NetTalkWithConn *)talks[i])->_conn, false);
    }
}
