/*
 * =====================================================================================
 *
 *       Filename:  push.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/15/2013 09:51:14 PM
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
#include "client.h"

Client client;

char req_buf[1024];

void *worker(void *args)
{
    NetPoller poller(&client);

    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7654);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    char res_buf[sizeof req_buf];
    while (1)
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        connect(sock, (struct sockaddr *)&addr, sizeof addr);

        NetTalk talk;

        talk._sock = sock;
        talk._status = 0;
        talk._errno = 0;

        talk._req_head._magic_num = MAGIC_NUM;

        talk._req_buf = req_buf;
        talk._req_len = sizeof req_buf;

        talk._res_buf = res_buf;
        talk._res_len = sizeof res_buf;

        poller.add(&talk, 1000);

        NetTalk *pt;
        int ret = poller.poll(&pt, 1, -1);
        if (ret > 0)
        {
            if (pt->_status == NET_ST_DONE)
                NOTICE("process ok, [len=%u]", pt->_res_head._body_len);
            else
            {
                WARNING("process fail, status=%d, errno=%d", pt->_status, pt->_errno);
            }
        }

        close(sock);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < sizeof req_buf/sizeof req_buf[0]; ++i)
        req_buf[i] = rand()%26 + (rand()%2 ? 'a' : 'A');

    client.init();

    pthread_t workers[30];
    for (int i = 0; i < 30; ++i)
        pthread_create(workers + i, NULL, worker, NULL);

    client.run();

    for (int i = 0; i < 30; ++i)
        pthread_join(workers[i], NULL);
    client.join();
    return 0;
}
