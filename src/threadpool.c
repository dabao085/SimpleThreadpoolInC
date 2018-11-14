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
* @file threadpool.c
* @brief implement of threadpool
*/

#include "threadpool.h"
#define true 1
#define false 0

/**
* @function threadpool_create
* @brief 创建线程池对象.
* @param thread_size 工作线程数.
* @param queue_size   队列大小.
* @成功是返回新建的threadpool_t对象指针, 失败返回NULL
*/
threadpool_t *threadpool_create(int thread_size, int queue_size)
{
    threadpool_t *pool = NULL;
    int i, ret = 0;
    int err_flag = false;

    if(thread_size < 0 || thread_size > MAXTHREADS || queue_size < 0 || queue_size > MAXQUEUES){
        printf("argument is not valiable\n");
        return NULL;    
    }

    do{
        pool = (threadpool_t*)malloc(sizeof(threadpool_t));
        if(pool == NULL){
            printf("malloc threadpool_t error\n");
            return NULL;
        }
        pool->pending_task_count = pool->head = pool->tail = pool->started = 0;
        pool->queue_size = queue_size;
        pool->thread_size = 0;//*
        pool->shutdown_flag = 0;

        pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_size);
        pool->queue = (threadpool_task_t*)malloc(sizeof(threadpool_task_t) * queue_size);

        if(pool->threads == NULL || pool->queue == NULL){
            printf("threads or queue in pool allocate error\n");
            err_flag = true;
            break;
        }

        if(pthread_mutex_init(&pool->mutex, NULL) != 0 || pthread_cond_init(&pool->notify, NULL) != 0){
            printf("threadpool lock failure\n");
            err_flag = true;
            break;
        }

        for(i = 0; i < thread_size; ++i){
            if((ret = pthread_create(&(pool->threads[i]), NULL, threadpool_func, (void*)pool)) != 0){
                threadpool_destroy(pool, 0);//工作线程在创建时失败，处理资源回收工作
                return NULL;
            }
            pool->thread_size ++;
            pool->started ++;
        }
    }while(0);

    if(err_flag){
        //工作线程创建之前发生错误，直接调用threadpool_free进行清理工作
        threadpool_free(pool);       
        return NULL; 
    }

    return pool;
}

/**
* @function threadpool_add
* @brief 向threadpool添加任务.
* @param pool 线程池指针.
* @param function 任务实现(任务对象为函数指针, 此处没有进行封装).
* @param inputArgument 传递给任务的入参数.
* @param outputArgument 传递给任务的结果.
* @成功返回0, 失败返回错误码(错误码定义见threadpool_error_code)
*/
int threadpool_add(threadpool_t *pool, void(*function)(void*, void*), void *inputArgument, void *outputArgument)
{
    int next = 0, err_code = 0;

    if(pool == NULL || function == NULL){
        return threadpool_invalid;
    }
    
    if(pthread_mutex_lock(&pool->mutex) != 0){
        return threadpool_lock_failure;
    }

    do{
        next = (pool->tail + 1) % pool->queue_size;

        //队列满了吗?
        if(pool->pending_task_count == pool->queue_size){
            err_code = threadpool_queue_full;
            break;
        }

        //线程池是否正在shutting down? 若为退出状态则拒绝接受新的任务
        if(pool->shutdown_flag){
            err_code = threadpool_shutdown;
            break;
        }

        //更新队列属性值, 添加任务
        pool->queue[pool->tail].inputArgument = inputArgument;
        pool->queue[pool->tail].outputArgument = outputArgument;
        pool->queue[pool->tail].function = function;
        pool->tail = next;
        pool->pending_task_count ++ ;

        //通知工作线程
        if(pthread_cond_signal(&pool->notify) != 0){
            err_code = threadpool_lock_failure;
            break;
        }
    }while(0);

    //unlock
    if(pthread_mutex_unlock(&pool->mutex) != 0){
        err_code = threadpool_lock_failure; //会遮盖上面已有的err_code,必然只能取其一,因为无法返回两个err_code
    }
    return err_code;
}

/**
* @function threadpool_func
* @brief 工作线程.
* @param threadpool 线程池指针.
* @由pthread_create调用
*/
void *threadpool_func(void *threadpool)
{
    threadpool_t *pool = threadpool;
    threadpool_task_t task;

    if(pool == NULL){
        printf("threadpool_invalid\n");
        return NULL;
    }

    for(;;){
        //使用条件变量之前必须加锁
        if(pthread_mutex_lock(&pool->mutex) != 0){
            printf("threadpool_lock_failure\n");
            return NULL;
        }

        //若没有未完成任务且线程池未关闭,则继续等待新任务添加
        while(pool->pending_task_count == 0 && !(pool->shutdown_flag)){
            if(pthread_cond_wait(&(pool->notify), &(pool->mutex)) != 0){
                printf("pthread_cond_wait error\n");
                return NULL;
            }
        }

        //检测线程池是否异常止,若正常停止,顺便检查下是否有未完成的任务在队列里
        if(pool->shutdown_flag == shutdown_immediate || (pool->shutdown_flag == shutdown_graceful && pool->pending_task_count == 0)){
            if(pool->shutdown_flag == shutdown_immediate){
                printf("pool->shutdown_flag == shutdown_immediate\n");
            }else if(pool->shutdown_flag == shutdown_graceful && pool->pending_task_count == 0){
                printf("pool->shutdown_flag == shutdown_graceful && pool->pending_task_count == 0 没有新任务, 正常退出\n");
            }
            break;
        }

        //抓取任务
        task.inputArgument = pool->queue[pool->head].inputArgument;
        task.outputArgument = pool->queue[pool->head].outputArgument;
        task.function = pool->queue[pool->head].function;
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->pending_task_count --;

        //解锁
        if(pthread_mutex_unlock(&(pool->mutex)) != 0){
            printf("threadpool_lock_failure\n");
            return NULL;
        }

        //执行任务
        (*(task.function))(task.inputArgument, task.outputArgument);
        //printf("task done...\n");
    }

    pool->started --;//表示某个工作线程挂啦(或者收到退出信号后,完成了所有任务后,正常退出),已启动的工作线程数减1
    if(pthread_mutex_unlock(&(pool->mutex)) != 0){
        printf("threadpool_lock_failure\n");
        return NULL;
    }
    pthread_exit(NULL);
    return NULL;
}

/**
* @function threadpool_free
* @brief 释放线程池等一众资源.
* @param pool 线程池指针.
* @成功返回0,失败返回错误码.
*/
int threadpool_free(threadpool_t *pool)
{
    if(pool == NULL){
        printf("threadpool_invalid\n");
        return threadpool_invalid;
    }

    if(pool->threads){
        free(pool->threads);
        free(pool->queue);

        //尝试lock一下,检查mutex是否正确初始化
        pthread_mutex_lock(&(pool->mutex));
        pthread_mutex_destroy(&(pool->mutex));
        pthread_cond_destroy(&(pool->notify));
    }
    free(pool);
    return 0;
}

/**
* @function threadpool_destroy
* @brief 关闭线程池, 按flag选择是否等待工作线程完成任务后退出.
* @param pool 线程池指针.
* @param flag 2表示立即退出(shutdown_immediate), 1表示等待工作线程完成后退出(shutdown_graceful).
* @成功返回0,失败返回错误码.
*/
int threadpool_destroy(threadpool_t *pool, int flag)
{
    int i, err_code = 0;
    if(pool == NULL){
        printf("threadpool_invalid\n");
        return threadpool_invalid;
    }

    if(pthread_mutex_lock(&(pool->mutex)) != 0){
        return threadpool_lock_failure;
    }

    do{
        //检查当前线程池是否正在关闭
        if(pool->shutdown_flag){
            err_code = threadpool_shutdown;
            break;
        }

        //若线程池不在关闭状态中,则置为关闭状态(关闭状态由flag指定)
        pool->shutdown_flag = (flag == shutdown_graceful )? shutdown_graceful:shutdown_immediate;

        //唤醒所有工作线程,感觉这样写有问题,会被中断吗?拆开写比较好
        if((pthread_cond_broadcast(&pool->notify) != 0) || (pthread_mutex_unlock(&(pool->mutex)) != 0)){
            err_code = threadpool_lock_failure;
            break;
        }

        //若为shutdown_graceful, 等待工作线程完成所有任务后退出, 若为shutdown_immediate则直接退出
        if(pool->shutdown_flag == shutdown_graceful){
            for(i = 0 ; i < pool->thread_size; ++ i){
                if(pthread_join(pool->threads[i], NULL) != 0){
                    err_code = threadpool_lock_failure;
                    break;
                }
            }
        }
    }while(0);

    //表示所有线程都已经停止,可以进行内存清理操作
    if(!err_code){
        threadpool_free(pool);
    }

    return err_code;
}