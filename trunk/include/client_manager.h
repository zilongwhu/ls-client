/*
 * =====================================================================================
 *
 *       Filename:  client_manager.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/12/2014 12:43:53 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#ifndef __LS_CLIENT_MANAGER_H__
#define __LS_CLIENT_MANAGER_H__

#include <pthread.h>
#include <vector>
#include "client.h"
#include "service_manager.h"

class ClientManager
{
    private:
        ClientManager(const ClientManager &o);
        ClientManager &operator =(const ClientManager &o);
    public:
        ClientManager() { _stopping = false; }
        ~ClientManager();

        int init(const char *path, const char *file);

        NetPoller *create_poller()
        {
            return new NetPoller(&_client);
        }
        void destroy_poller(NetPoller *poller)
        {
            if (poller)
            {
                delete poller;
            }
        }

        int post_request(const char *service_name, NetTalkWithConn *talk, int timeout, NetPoller *poller = NULL);
        int poll(NetTalkWithConn **talks, int count, int timeout_ms, NetPoller *poller = NULL);
        void cancel(NetTalkWithConn *talk, NetPoller *poller = NULL);
        void cancelAll(NetPoller *poller = NULL);

        int request(const char *service_name,
                void *req, unsigned int req_len,
                void *res, unsigned int &res_len,
                int timeout);
    private:
        NetPoller *get_ts_poller();
        static void *healthy_checker(void *args)
        {
            ClientManager *manager = (ClientManager *)args;
            while (!manager->_stopping)
            {
                sleep(1);
                manager->_services.check_healthy();
            }
            return NULL;
        }
    private:
        Client _client;
        ServiceManager _services;
        pthread_key_t _poller_key;
        Mutex _mutex;
        std::vector<NetPoller *> _pollers;
        volatile bool _stopping;
        pthread_t _checker_tid;
};

#endif
