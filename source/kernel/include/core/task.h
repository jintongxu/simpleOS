/**
 * 任务实现
 */
#ifndef TASK_H
#define TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/list.h"
#include "fs/file.h"

#define TASK_NAME_SIZE      32                      // 任务名字长度
#define TASK_TIME_SLICE_DEFAULT         10          // 时间片计数
#define TASK_OFILE_NR       128                     // 最多支持打开的文件数量

#define TASK_FLAGS_SYSTEM       (1 << 0)            // 系统任务

typedef struct _task_args_t {
    uint32_t ret_addr;              // 返回地址，无用
    uint32_t argc;
    char ** argv;
}task_args_t;


/**
 * @brief 任务控制块结构
 */
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

    int pid;                        // 进程的pid
    struct _task_t * parent;        // 父进程
    uint32_t heap_start;            // 堆的顶层地址
    uint32_t heap_end;              // 堆结束地址
    int status;                     // 进程执行结果

    int sleep_ticks;        // 睡眠时间
    int time_ticks;         // 设置计数器   时间片
    int slice_ticks;        // 递减时间片计数

    file_t * file_table[TASK_OFILE_NR];      // 记录进程打开了哪些文件  任务最多打开的文件数量

    char name[TASK_NAME_SIZE];      // 任务名字

    list_node_t run_node;           // 运行相关结点
    list_node_t wait_node;          // 等待队列
    list_node_t all_node;           // 所有队列结点

    tss_t tss;                  // 任务的TSS段
    int tss_sel;                // tss选择子
}task_t;


int task_init (task_t * task, const char * name, int flag,uint32_t entry, uint32_t esp);
void task_switch_from_to (task_t * from, task_t * to);
void task_time_tick(void);

file_t * task_file (int fd);            // 根据进程的 file_table 找到对应的文件描述符
int task_alloc_fd (file_t * file);      // 在进程中分配文件id--进程打开的文件  在进程的 file_table 找到空闲的
void task_remove_fd (int fd);           // 释放进程绑定的文件id


typedef struct _task_manager_t {
    task_t * curr_task;     // 当前运行的任务

    list_t ready_list;      // 就绪队列
    list_t task_list;       // 保存所有已经创建好的进程 所有已创建任务的队列
    list_t sleep_list;      // 睡眠队列 延时队列

    task_t first_task;      // 内核任务
    task_t idle_task;       // 空闲任务

    int app_code_sel;       // 任务代码段选择子
    int app_data_sel;       // 应用任务的数据段选择子
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