/*
* Copyright (c) 2018, Leonardo Cheng <chengxiang085@gmail.com>.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    threadpool_t *pool;
    int inputArgument[128] = {0}, outputArgument[128] = {0};
    pthread_mutex_init(&g_lock, NULL);
    pool = threadpool_create(1, 96);
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
    for(int i = 0; i < 96; ++i){
        printf("%d ", outputArgument[i]);
    }
    printf("\n");
    return 0;
}