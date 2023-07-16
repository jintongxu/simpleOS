#ifndef TASK_H
#define TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/list.h"

#define TASK_NAME_SIZE      32

typedef struct _task_t {
    // uint32_t * stack;
    // 这是个枚举数据类型，递增的宏定义，默认第一个为0，每次加1.
    enum {
        TASK_CREATED,
        TASK_RUNNING,
        TASK_SLEEP,
        TASK_READY,
        TASK_WAITTING,   // 等待时间
    }state;

    char name[TASK_NAME_SIZE];

    list_node_t run_node;
    list_node_t all_node;

    tss_t tss;
    int tss_sel;
}task_t;


int task_init (task_t * task, const char * name, uint32_t entry, uint32_t esp);
void task_switch_from_to (task_t * from, task_t * to);

typedef struct _task_manager_t {
    task_t * curr_task;

    list_t ready_list;  // 就绪队列
    list_t task_list;   // 保存所有已经创建好的进程

    task_t first_task;
}task_manager_t;


void task_manager_init (void);
void task_first_init (void);
task_t * task_first_task (void);
void task_set_ready(task_t * task);
void task_set_block (task_t * task);
int sys_sched_yield(void);
void task_dispatch (void);
task_t * task_current (void);



#endif