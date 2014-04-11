/*
 * =====================================================================================
 *
 *       Filename:  server.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/11/2014 04:13:14 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#ifndef __LS_CLIENT_SERVER_H__
#define __LS_CLIENT_SERVER_H__

#include <deque>
#include <string>
#include <sys/socket.h>
#include "lock.h"

class Server
{
    private:
        Server(const Server &o);
        Server &operator =(const Server &o);
    public:
        Server()
        {
            ::bzero(&_addr, sizeof _addr);
            _addrlen = 0;
            _connect_timeout = -1;

            ::bzero(_stats, sizeof _stats);
            _stats_off = 0;
            _fail_count = 0;
            _pool_size = 0;
        }
        ~Server();

        void set_pool_size(int sz) { _pool_size = sz; }
        void set_connect_timeout(int ms) { _connect_timeout = ms; }
        int set_addr(const char *ptr, int port);

        double fail_ratio() const
        {
            return _fail_count * 1.0 / (sizeof(_stats)/sizeof(_stats[0]));
        }

        int get_avail_sock();
        void return_sock(int sock, bool is_ok);
    protected:
        Mutex _mutex;
        std::deque<int> _socks;
        int _pool_size;

        struct sockaddr_storage _addr;
        socklen_t _addrlen;
        std::string _addrstr;
        int _connect_timeout;

        int _stats_off;
        volatile int _fail_count;
        int8_t _stats[2048];
};

#endif