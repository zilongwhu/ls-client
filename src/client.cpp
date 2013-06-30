/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2012年05月23日 15时24分57秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zilong.Zhu (), zilong.whu@gmail.com
 *        Company:  edu.whu
 *
 * =====================================================================================
 */

#include <new>
#include "log.h"
#include "client.h"

static void *worker(void *arg)
{
    ClientEpex *ep = (ClientEpex *)arg;
    ep->run();
    return NULL;
}

int Client::init(int wn)
{
    if (wn <= 0)
        wn = 1;
    if (_worker_num > 0)
        return 0;
    _tids = new(std::nothrow) pthread_t[wn];
    _workers = new(std::nothrow) ClientEpex[wn];
    if (NULL == _tids || NULL == _workers)
        goto FAIL;
    for (int i = 0; i < wn; ++i)
    {
        if (_workers[i].init() < 0)
            goto FAIL;
    }
    _worker_num = wn;
    return 0;
FAIL:
    if (_tids)
    {
        delete [] _tids;
        _tids = NULL;
    }
    if (_workers)
    {
        delete [] _workers;
        _workers = NULL;
    }
    return -1;
}

int Client::run()
{
    int i;
    for (i = 0; i < _worker_num; ++i)
    {
        int ret = pthread_create(_tids + i, NULL, worker, _workers + i);
        if (ret != 0)
        {
            WARNING("failed to create worker thread[%d], err=%d", i, ret);
            break;
        }
    }
    return (_worker_num = i);
}

void Client::stop()
{
    for (int i = 0; i < _worker_num; ++i)
    {
        _workers[i].stop();
    }
    for (int i = 0; i < _worker_num; ++i)
    {
        pthread_join(_tids[i], NULL);
    }
}
