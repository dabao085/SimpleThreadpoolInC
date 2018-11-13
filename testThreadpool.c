/**
* @file testThreadpool.c
* @brief 简单的线程池测试程序
*/

#include "threadpool.h"
#include <assert.h>

int g_done = 0, g_started = 0;
pthread_mutex_t g_lock;

void testThread(void *inputArgument, void *outputArgument)
{
    pthread_mutex_lock(&g_lock);
    g_done++;
    printf("g_done now is : %d\n", g_done);
    int *p = (int*)inputArgument;
    *p = *p + 100;
    *(int*)outputArgument = *p;
    pthread_mutex_unlock(&g_lock);
}

int main(int argc, char **argv)
{
    int i;
    threadpool_t *pool;
    int inputArgument[128] = {0}, outputArgument[128] = {0};
    pthread_mutex_init(&g_lock, NULL);
    pool = threadpool_create(4, 96);
    assert(pool != NULL);

    printf("threadpool woker size: %d, task size: %d\n", 8, 64);

    while(threadpool_add(pool, &testThread, (void*)&inputArgument[g_started], (void*)&outputArgument[g_started]) == 0){
        pthread_mutex_lock(&g_lock);
        g_started++;
        inputArgument[g_started] = g_started;
        pthread_mutex_unlock(&g_lock);
    }

    assert(threadpool_destroy(pool, 1) == 0);
    printf("g_start: %d, g_done: %d\n", g_started, g_done);

    printf("result:\n");
    for(i = 0; i < 96; ++i){
        printf("%d ", outputArgument[i]);
    }
    printf("\n");
    return 0;
}