/**
* @file testPress.c
* @brief 简单的线程池压力测试
*/
#include "threadpool.h"
#include <assert.h>

#define QUEUE 8192
#define THREAD 4
#define SIZE 64

int g_started, g_done, g_left;
threadpool_t *pool[SIZE];
pthread_mutex_t lock;

void testThread(void *inputArgument, void *outputArgument)
{
    usleep(20);
//    printf("g_done %d\n", ++g_done);
    int *p = (int*)inputArgument;
    *p = *p + 1;
    if(*p < SIZE){
        assert(threadpool_add(pool[*p], &testThread, inputArgument, NULL) == 0);
    }else{
        pthread_mutex_lock(&lock);
        g_left--;
        pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char **argv)
{
    int i, j;
    g_started = g_done = 0;
    int argument[QUEUE];
    g_left = QUEUE;
    pthread_mutex_init(&lock, NULL);
    
    //创建64个线程池
    for(i = 0; i < SIZE; ++i){
        pool[i] = threadpool_create(THREAD, QUEUE);
        assert(pool[i] != NULL);
    }

    //添加任务
    for(i = 0; i < QUEUE; ++i){
        argument[i] = 0;
        assert(threadpool_add(pool[0], &testThread, (void*)&argument[i], NULL) == 0);
    }

    while(copy > 0){
        usleep(10);
        pthread_mutex_lock(&lock);
        g_left;
        printf("left tasks is %d\n", g_left);
        pthread_mutex_unlock(&lock);
    }

    for(i = 0; i < SIZE; ++i){
        assert(threadpool_destroy(pool[i], shutdown_graceful) == 0);
    }
    pthread_mutex_destroy(&lock);
    return 0;
}