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

#include "client.h"
#include "service_manager.h"

class ClientManager
{
    private:
        ClientManager(const ClientManager &o);
        ClientManager &operator =(const ClientManager &o);
    public:
        ClientManager() { }
        ~ClientManager() { }

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

        int post_request(const char *service_name, NetTalkWithConn *talk, int timeout, NetPoller *poller);
        int poll(NetTalkWithConn **talks, int count, int timeout_ms, NetPoller *poller);
        void cancel(NetTalkWithConn *talk, NetPoller *poller);
        void cancelAll(NetPoller *poller);
    private:
        Client _client;
        ServiceManager _services;
};

#endif
