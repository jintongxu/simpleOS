#include "core/task.h"
#include "tools/klib.h"
#include "os_cfg.h"
#include "cpu/cpu.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"

static task_manager_t task_manager;

static int tss_init (task_t * task, uint32_t entry, uint32_t esp) {
    int tss_sel = gdt_alloc_desc();
    if (tss_sel < 0) {
        log_printf("alloc tss failed.\n");
        return -1;
    }

    segment_desc_set(tss_sel, (uint32_t)&task->tss, sizeof(tss_t),
        SEG_P_PRESENT | SEG_DPL0 | SEG_TYPE_TSS
    );
    
    

    kernel_memset(&task->tss, 0, sizeof(tss_t));
    task->tss.eip = entry;
    task->tss.esp = task->tss.esp0 = esp;
    task->tss.ss = task->tss.ss0 = KERNEL_SELECTOR_DS;
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = KERNEL_SELECTOR_DS;
    task->tss.cs = KERNEL_SELECTOR_CS;
    task->tss.eflags = EFLGAGS_IF | EFLGAGS_DEFAULT;
    
    task->tss_sel = tss_sel;
    return 0;
}

int task_init (task_t * task, const char * name, uint32_t entry, uint32_t esp) {
    ASSERT(task != (task_t *)0);

    tss_init(task, entry, esp);

    kernel_strncpy(task->name, name, TASK_NAME_SIZE);
    task->state = TASK_CREATED;
    list_node_init(&task->all_node);
    list_node_init(&task->run_node);


    task_set_ready(task);
    list_insert_last(&task_manager.task_list, &task->all_node);

    // uint32_t * pesp = (uint32_t *)esp;
    // if (pesp) {
    //     *(--pesp) = entry;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     task->stack = pesp;

    // }

    
    return 0;
}

void simple_switch (uint32_t **from, uint32_t * to);

// 简单的用jmp到对应的tss选择子进行任务切换
void task_switch_from_to (task_t * from, task_t * to) {
    switch_to_tss(to->tss_sel);
    // simple_switch(&from->stack, to->stack);
}

void task_first_init (void) {
    task_init(&task_manager.first_task, "fist task" ,0, 0);  
    write_tr(task_manager.first_task.tss_sel);
    task_manager.curr_task = &task_manager.first_task; 
}

task_t * task_first_task (void) {
    return &task_manager.first_task;
}


void task_manager_init (void) {
    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);
    task_manager.curr_task = (task_t *)0;

}

void task_set_ready(task_t * task) {
    list_insert_last(&task_manager.ready_list, &task->run_node);
    task->state = TASK_READY;
}


void task_set_block (task_t * task) {
    list_remove(&task_manager.ready_list, &task->run_node);
}


// 返回下一个的进程(就绪队列头部的进程)
task_t * task_next_run (void) {
    list_node_t * task_node = list_first(&task_manager.ready_list);
    return list_node_parent(task_node, task_t, run_node);
}


task_t * task_current (void) {
    return task_manager.curr_task;
}

// 让进程让出CPU
int sys_sched_yield() {
    if (list_count(&task_manager.ready_list) > 1) {
        task_t * curr_task = task_current();

        task_set_block(curr_task);
        task_set_ready(curr_task);    // 再加入的时候，是加入队列的尾部

        task_dispatch();
    }

    // 如果就绪队列里面就1个进程。
    return 0;
}


void task_dispatch (void) {
    task_t * to = task_next_run();
    if (to != task_manager.curr_task) {
        task_t * from = task_current();
        task_manager.curr_task = to;
        to->state = TASK_RUNNING;

        task_switch_from_to(from, to);
    }
}
