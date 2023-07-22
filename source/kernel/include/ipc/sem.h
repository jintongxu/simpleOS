#ifndef SEM_H
#define SEM_H

#include "tools/list.h"

typedef struct _sem_t {
    int count;
    list_t wait_list;
}sem_t;


void sem_init (sem_t * sem, int init_count);
void sem_wait (sem_t * sem);   // 等信号
void sem_notify (sem_t * sem);   // 发信号

#endif