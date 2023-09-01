/**
 * 互斥锁
 */
#ifndef MUTEX_H
#define MUTEX_H

#include "core/task.h"
#include "tools/list.h"

/**
 * 进程同步用的计数信号量
 */
typedef struct _mutex_t {

    task_t * owner;     // 锁的拥有者
    int locked_count;     // 上锁了多少次
    list_t wait_list;
}mutex_t;


void mutex_init (mutex_t * mutex);
void mutex_lock (mutex_t * mutex);
void mutex_unlock (mutex_t * mutex);
#endif