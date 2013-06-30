/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  客户端
 *
 *        Version:  1.0
 *        Created:  06/29/2013 02:15:13 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *   Organization:  edu.whu
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.h"
#include "exnet.h"
#include "client.h"
#include "net_poller.h"

int main(int argc, char *argv[])
{
    Client client;
    client.init();
    NetPoller poller(&client);

    client.run();

    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7654);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int sock2 = socket(AF_INET, SOCK_STREAM, 0);
    connect(sock, (struct sockaddr *)&addr, sizeof addr);
    connect(sock2, (struct sockaddr *)&addr, sizeof addr);

    char req_buf[] = "ABcdefG HI JK lmN";
    char res_buf[sizeof req_buf];

    NetTalk talk;
    NetTalk talk2;

    talk._sock = sock;
    talk._status = 0;
    talk._errno = 0;

    talk._req_head._magic_num = MAGIC_NUM;

    talk._req_buf = req_buf;
    talk._req_len = sizeof req_buf;

    talk._res_buf = res_buf;
    talk._res_len = sizeof res_buf;

    talk2._sock = sock2;
    talk2._status = 0;
    talk2._errno = 0;

    talk2._req_head._magic_num = MAGIC_NUM;

    talk2._req_buf = req_buf;
    talk2._req_len = sizeof req_buf;

    talk2._res_buf = res_buf;
    talk2._res_len = sizeof res_buf;

    poller.add(&talk, 1200);
    poller.add(&talk2, 1600);

    usleep(100);
    poller.cancelAll();
    close(sock);

    NetTalk *pt;
    int ret = poller.poll(&pt, 1, -1);
    if (ret > 0)
    {
        if (pt->_status == NET_ST_DONE)
            NOTICE("[len=%u][%s]\n", pt->_res_head._body_len, res_buf);
        else
            NOTICE("status=%d, errno=%d\n", pt->_status, pt->_errno);
    }
    close(sock2);

    client.join();
    return 0;
}

#ifdef __ABC_TEST__
int main(int argc, char *argv[])
{
	netresult_t result;
	char buffer[] = "hello";
	char buffer2[256];

	epex_t handle = epex_open(1024);
	if ( NULL == handle )
	{
		return -1;
	}
	struct sockaddr_in addr;
	bzero(&addr, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if ( sock < 0 )
	{
		return -1;
	}
	int first = 1;
	if ( epex_connect(handle, sock, &addr, NULL, 100) )
	{
		ssize_t sz;
		do
		{
			sz = epex_poll(handle, &result, 1);
			if ( 1 != sz )
			{
				continue;
			}
			switch (result._op_type)
			{
				case NET_OP_CONNECT:
					if ( NET_DONE == result._status )
					{
						NOTICE("connect ok");
						if ( !epex_write(handle, sock,
									"POST /index.html http/1.1\r\ntransfer-encoding:chunked\r\n\r\n\r\n   \r",
									sizeof("POST /index.html http/1.1\r\ntransfer-encoding:chunked\r\n\r\n\r\n   \r") - 1,
									NULL, -1) )
						{
							goto OUT;
						}
						NOTICE("submit write request[%s]", buffer);
					}
					else
					{
						goto OUT;
					}
					break;
				case NET_OP_WRITE:
					if ( NET_DONE == result._status )
					{
						NOTICE("submit read request");
						usleep(50*1000);
						epex_write(handle, sock,
									"\n 00010 \"\r\n \\\"a\r\n\"\r\n0123456789ABCDEF\r\n 001\r\nL\r\n 0 \r\n first : abc\r\n efg\r\nnext  :  \r\n every    one   \r\n\r\n",
									sizeof("\n 00010 \"\r\n \\\"a\r\n\"\r\n0123456789ABCDEF\r\n 001\r\nL\r\n 0 \r\n first : abc\r\n efg\r\nnext  :  \r\n every    one   \r\n\r\n") - 1,
									NULL, -1);
						if ( 0 == first )
						{
							goto OUT;
						}
						else
						{
							first = 0;
						}
					}
					else
					{
						goto OUT;
					}
					break;
			}
		} while(1);
OUT:
		;
	}
	epex_close(handle);
	return 0;
}
#endif
