/**
* @file testShutdown.c
* @brief 简单的线程池shutdown操作
*/

#include "threadpool.h"
#include <assert.h>

#define THREAD 8
#define QUEUE 64

pthread_mutex_t lock;
int g_left; //未完成的任务

void testThread(void *inputArgument, void *outputArgument)
{
    usleep(100);
    pthread_mutex_lock(&lock);
    --g_left;
    pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
    int i;
    g_left = QUEUE;
    pthread_mutex_init(&lock, NULL);
    threadpool_t *pool = threadpool_create(THREAD, QUEUE);

    if(pool == NULL){
        printf("threadpool_create failed\n");
        return -1;
    }

    //测试immediate shutdown
    printf("testing immediate shutdown\n");
    for(i = 0; i < QUEUE; ++i){
        if(threadpool_add(pool, testThread, NULL, NULL) != 0){
            printf("threadpool_add error\n");
            return -1;
        }
    }

    assert(threadpool_destroy(pool, shutdown_immediate) == 0);
    //shutdown_immediate后会残余部分任务未完成,此处检查是否有任务残留
    assert(g_left > 0);

    //测试graceful shutdown
    printf("testing graceful shutdown\n");
    pool = threadpool_create(THREAD, QUEUE);

    if(pool == NULL){
        printf("threadpool_create failed\n");
        return -1;
    }    
    for(i = 0; i < QUEUE; ++i){
        if(threadpool_add(pool, testThread, NULL, NULL) != 0){
            printf("threadpool_add error\n");
            return -1;
        }
    }

    assert(threadpool_destroy(pool, shutdown_graceful) == 0);
    //shutdown_graceful会等待所有未完成的任务完成,所以此时g_left应为0
    assert(g_left == 0);

    pthread_mutex_destroy(&lock);
    return 0;
}