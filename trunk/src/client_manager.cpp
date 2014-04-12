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

ClientManager::~ClientManager()
{
    _stopping = true;
    pthread_join(_checker_tid, NULL);
    pthread_key_delete(_poller_key);
    for (size_t i = 0; i < _pollers.size(); ++i)
    {
        delete _pollers[i];
    }
    _pollers.clear();
}

int ClientManager::init(const char *path, const char *file)
{
    if (NULL == path || NULL == file)
    {
        WARNING("invalid args: path=%p, file=%p", path, file);
        return -1;
    }
    int ret = pthread_key_create(&_poller_key, NULL);
    if (ret)
    {
        WARNING("failed to create _poller_key, error=%d", ret);
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
    ret = pthread_create(&_checker_tid, NULL, healthy_checker, this);
    if (ret)
    {
        WARNING("failed to create healthy checker thread, error=%d", ret);
        return -1;
    }
    WARNING("init client manager ok");
    return 0;
}

int ClientManager::post_request(const char *service_name, NetTalkWithConn *talk, int timeout, NetPoller *poller)
{
    if (NULL == service_name || NULL == talk)
    {
        WARNING("invalid args: service_name=%p, talk=%p", service_name, talk);
        return -1;
    }
    if (NULL == poller)
    {
        poller = this->get_ts_poller();
        if (NULL == poller)
        {
            WARNING("failed to get ts poller");
            return -1;
        }
    }
    if (_services.get_connection(service_name, talk->_conn) < 0)
    {
        WARNING("failed to get connection from service[%s]", service_name);
        return -1;
    }
    talk->_sock = talk->_conn._sock;
    if (!poller->add(talk, timeout))
    {
        talk->_sock = -1;
        _services.return_connection(talk->_conn, true);
        WARNING("failed to add to poller");
        return -1;
    }
    return 0;
}

int ClientManager::poll(NetTalkWithConn **talks, int count, int timeout_ms, NetPoller *poller)
{
    if (NULL == talks || count < 0)
    {
        WARNING("invalid args: talks=%p, count=%d", talks, count);
        return -1;
    }
    if (NULL == poller)
    {
        poller = this->get_ts_poller();
        if (NULL == poller)
        {
            WARNING("failed to get ts poller");
            return -1;
        }
    }
    int ret = poller->poll((NetTalk **)talks, count, timeout_ms);
    for (int i = 0; i < ret; ++i)
    {
        talks[i]->_sock = -1;
        _services.return_connection(talks[i]->_conn, talks[i]->_status == NET_ST_DONE);
    }
    return ret;
}

void ClientManager::cancel(NetTalkWithConn *talk, NetPoller *poller)
{
    if (NULL == talk)
    {
        WARNING("invalid args: talk=%p", talk);
        return ;
    }
    if (NULL == poller)
    {
        poller = this->get_ts_poller();
        if (NULL == poller)
        {
            WARNING("failed to get ts poller");
            return ;
        }
    }
    poller->cancel(talk);
    talk->_sock = -1;
    _services.return_connection(talk->_conn, false);
}

void ClientManager::cancelAll(NetPoller *poller)
{
    if (NULL == poller)
    {
        poller = this->get_ts_poller();
        if (NULL == poller)
        {
            WARNING("failed to get ts poller");
            return ;
        }
    }
    std::vector<NetTalk *> talks;
    poller->getTalks(talks);
    poller->cancelAll();
    for (size_t i = 0; i < talks.size(); ++i)
    {
        talks[i]->_sock = -1;
        _services.return_connection(((NetTalkWithConn *)talks[i])->_conn, false);
    }
}

NetPoller *ClientManager::get_ts_poller()
{
    NetPoller *poller = (NetPoller *)pthread_getspecific(_poller_key);
    if (NULL == poller)
    {
        poller = this->create_poller();
        if (NULL == poller)
        {
            WARNING("failed to create poller");
            return NULL;
        }
        int ret = pthread_setspecific(_poller_key, poller);
        if (ret)
        {
            WARNING("pthread_setspecific failed, error=%d", ret);
            this->destroy_poller(poller);
            return NULL;
        }
        AutoLock __lock(_mutex);
        _pollers.push_back(poller);
    }
    return poller;
}

int ClientManager::request(const char *service_name,
        void *req, unsigned int req_len,
        void *res, unsigned int &res_len,
        int timeout)
{
    if (NULL == service_name || NULL == req || NULL == res)
    {
        WARNING("invalid args: service_name=%p, req=%p, res=%p", service_name, req, res);
        return -1;
    }
    NetTalkWithConn talk;
    ::bzero(&talk, sizeof(talk));
    talk._req_buf = req;
    talk._req_len = req_len;
    talk._res_buf = res;
    talk._res_len = res_len;

    NetPoller poller(&_client);
    int ret = this->post_request(service_name, &talk, timeout, &poller);
    if (ret < 0)
    {
        WARNING("failed to post request");
        return -1;
    }
    NetTalkWithConn *talks;
    ret = poll(&talks, 1, -1, &poller);
    if (ret != 1 || talks != &talk)
    {
        WARNING("something wrong here?");
        return -1;
    }
    if (talk._status != NET_ST_DONE)
    {
        WARNING("talk failed, status=%d, errno=%d", talk._status, talk._errno);
        return -1;
    }
    res_len = talk._res_head._body_len;
    return 0;
}
