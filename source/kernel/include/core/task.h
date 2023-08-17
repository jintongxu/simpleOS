#ifndef TASK_H
#define TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/list.h"
#include "fs/file.h"

#define TASK_NAME_SIZE      32
#define TASK_TIME_SLICE_DEFAULT         10
#define TASK_OFILE_NR       128

#define TASK_FLAGS_SYSTEM       (1 << 0)

typedef struct _task_args_t {
    uint32_t ret_addr;
    uint32_t argc;
    char ** argv;
}task_args_t;
typedef struct _task_t {
    // uint32_t * stack;
    // 这是个枚举数据类型，递增的宏定义，默认第一个为0，每次加1.
    enum {
        TASK_CREATED,
        TASK_RUNNING,
        TASK_SLEEP,
        TASK_READY,
        TASK_WAITTING,   // 等待时间
        TASK_ZOMBIE,    // 将死状态
    }state;

    int pid;
    struct _task_t * parent;
    uint32_t heap_start;    // 堆的起始地址
    uint32_t heap_end;
    int status;         // 进程执行结果

    int sleep_ticks;
    int time_ticks;      // 设置计数器
    int slice_ticks;

    file_t * file_table[TASK_OFILE_NR];      // 记录进程打开了哪些文件

    char name[TASK_NAME_SIZE];

    list_node_t run_node;
    list_node_t wait_node;
    list_node_t all_node;

    tss_t tss;
    int tss_sel;
}task_t;


int task_init (task_t * task, const char * name, int flag,uint32_t entry, uint32_t esp);
void task_switch_from_to (task_t * from, task_t * to);
void task_time_tick(void);

file_t * task_file (int fd);            // 根据进程的 file_table 找到对应的文件描述符
int task_alloc_fd (file_t * file);      // 在进程中分配文件id--进程打开的文件  在进程的 file_table 找到空闲的
void task_remove_fd (int fd);           // 释放进程绑定的文件id


typedef struct _task_manager_t {
    task_t * curr_task;

    list_t ready_list;  // 就绪队列
    list_t task_list;   // 保存所有已经创建好的进程
    list_t sleep_list;   // 睡眠队列

    task_t first_task;
    task_t idle_task;

    int app_code_sel;       // 代码段选择子
    int app_data_sel;       // 数据段选择子
}task_manager_t;


void task_manager_init (void);
void task_first_init (void);
task_t * task_first_task (void);
void task_set_ready(task_t * task);
void task_set_block (task_t * task);
int sys_sched_yield(void);
void task_dispatch (void);
task_t * task_current (void);

void task_set_sleep (task_t * task, uint32_t ticks);   // 将进程加入睡眠队列
void task_set_wakeup (task_t * task);       // 将进程从睡眠队列移除

void sys_sleep (uint32_t ms);
int sys_getpid (void);
int sys_fork (void);
int sys_execve (char * name, char **argv, char ** env);

void sys_exit (int status);
int sys_wait (int * status);

#endif