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

#include "server.h"

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

        int init(const char *path, const char *file);
        int init(const std::vector<server_args> &args);
    private:
        Server *_servers;
        int _server_num;
};

#endif
