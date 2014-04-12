/*
 * =====================================================================================
 *
 *       Filename:  service_manager.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/11/2014 11:32:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#ifndef __LS_CLIENT_SERVICE_MANAGER_H__
#define __LS_CLIENT_SERVICE_MANAGER_H__

#include <map>
#include <string>
#include "service.h"

class ServiceManager
{
    private:
        ServiceManager(const ServiceManager &o);
        ServiceManager &operator =(const ServiceManager &o);
    public:
        ServiceManager() { }
        ~ServiceManager() { this->clear(); }

        void clear();
        int init(const char *path, const char *file);
        int get_connection(const char *service_name, Connection &conn);
        void return_connection(Connection &conn, bool is_ok)
        {
            if (conn._server)
            {
                conn._server->return_sock(conn._sock, is_ok);
            }
        }
    private:
        std::map<std::string, Service *> _service_map;
};

#endif
