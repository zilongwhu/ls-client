/*
 * =====================================================================================
 *
 *       Filename:  connection.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/12/2014 09:15:19 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#ifndef __LS_CLIENT_CONNECTION_H__
#define __LS_CLIENT_CONNECTION_H__

#include "server.h"

struct Connection
{
    int _key;
    int _sock;
    Server *_server;
};

#endif
