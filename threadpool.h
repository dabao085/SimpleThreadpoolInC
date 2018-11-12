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

/**
* @file threadpool.h
* @brief threadpool header
*/

#ifndef _THREADPOLL_H_
#define _THREADPOLL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAXTHREADS 128
#define MAXQUEUES 128

/**
*  @struct threadpool_task_t
*  @brief the work task struct
*
*  @var function Pointer to the function that will perform the task.
*  @var argument Argument to be passed to the function.
*/
typedef struct{
    void (*function)(void *, void *);
    void *inputArgument;
    void *outputArgument;
}threadpool_task_t;

/**
*  @struct threadpool_t
*  @brief the threadpool struct
*
*  @var mutex 互斥量
*  @var notify 条件变量
*  @var threads 工作线程结构体指针
*  @var queue 任务队列
*  @var thread_size 工作线程数量
*  @var queue_size 队列中的任务数
*  @var head 指向任务队列中未完成任务的head
*  @var tail 指向任务队列中未完成任务的tail
*  @var shutdown_flag 线程池的退出标记
*  @var started 启动的工作线程数
*  @var pending_task_count 未完成的task数
*/
typedef struct{
    pthread_mutex_t mutex;
    pthread_cond_t notify;
    pthread_t *threads;
    threadpool_task_t *queue;
    int thread_size;
    int queue_size;
    int head;
    int tail;
    int shutdown_flag;
    int started;
    int pending_task_count; //count
}threadpool_t;

enum {
    threadpool_invalid = -1,
    threadpool_lock_failure = -2,
    threadpool_shutdown = -3,
    threadpool_queue_full = -4,
    threadpool_thread_failure = 5
}threadpool_error_code;

enum{
    shutdown_graceful = 1,
    shutdown_immediate = 2
}threadpool_shutdown_flag;

threadpool_t *threadpool_create(int thread_size, int queue_size);
int threadpool_add(threadpool_t *pool, void(*function)(void*, void*), void *inputArgument, void *outputArgument);
int threadpool_free(threadpool_t *pool);
int threadpool_destroy(threadpool_t *pool, int flag);
void *threadpool_func(void *threadpool);
#endif