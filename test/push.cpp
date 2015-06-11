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

    char *res_buf = new char[sizeof(req_buf)*128];
    while (1)
    {
        int num = 0;
        int socks[128];
        NetTalk talk[128];

        NOTICE("start to process.");
        for (size_t i = 0; i < sizeof socks/sizeof socks[0]; ++i)
        {
            socks[i] = socket(AF_INET, SOCK_STREAM, 0);
            if (socks[i] < 0)
                break;
            if (connect(socks[i], (struct sockaddr *)&addr, sizeof addr) < 0)
            {
                close(socks[i]);
                break;
            }

            talk[i]._sock = socks[i];
            talk[i]._status = 0;
            talk[i]._errno = 0;

            talk[i]._req_head._magic_num = MAGIC_NUM;

            talk[i]._req_buf = req_buf;
            talk[i]._req_len = sizeof req_buf;

            talk[i]._res_buf = res_buf + i*sizeof(req_buf);
            talk[i]._res_len = sizeof req_buf;

            if (!poller.add(talk + i, 1000))
            {
                close(socks[i]);
                break;
            }
            ++num;
        }

        NetTalk *pt[128];
        int ret = poller.poll(pt, num, -1);
        if (ret != num)
        {
            FATAL("ret[%d] != num[%d].", ret, num);
        }
        else for(int i = 0; i < ret; ++i)
        {
            if (pt[i]->_status == NET_ST_DONE)
                NOTICE("process sock[%d] ok, [len=%u]", pt[i]->_sock, pt[i]->_res_head._body_len);
            else
                WARNING("process sock[%d] fail, status=%d, errno=%d", pt[i]->_sock, pt[i]->_status, pt[i]->_errno);
        }
        for (int i = 0; i < num; ++i)
        {
            close(socks[i]);
        }
        NOTICE("end of process, num=%d.", num);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    for (size_t i = 0; i < sizeof req_buf/sizeof req_buf[0]; ++i)
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
