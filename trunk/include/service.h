/*
 * =====================================================================================
 *
 *       Filename:  service.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/11/2014 11:10:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#ifndef __LS_CLIENT_SERVICE_H__
#define __LS_CLIENT_SERVICE_H__

#include <vector>
#include <string>
#include "server.h"
#include "connection.h"

class Service
{
    private:
        Service(const Service &o);
        Service &operator =(const Service &o);
    public:
        Service()
        {
            _servers = NULL;
            _server_num = 0;
        }
        ~Service()
        {
            if (_servers)
            {
                delete [] _servers;
                _servers = NULL;
            }
            _server_num = 0;
        }

        const char *name() const { return _name.c_str(); }

        int init(const char *path, const char *file);
        int init(const std::string &name, const std::vector<server_args> &args);
        int get_connection(Connection &conn);
        void return_connection(Connection &conn, bool is_ok)
        {
            if (conn._server)
            {
                conn._server->return_sock(conn._sock, is_ok);
            }
        }
        void check_healthy();
    private:
        Server *_servers;
        int _server_num;
        std::string _name;
};

#endif
