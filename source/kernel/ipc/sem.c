/**
 * 计数信号量
 */
#include "ipc/sem.h"
#include "core/task.h"
#include "cpu/irq.h"

/**
 * 计数信号量
 *
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
/**
 * 信号量初始化
 */
void sem_init (sem_t * sem, int init_count) {
    sem->count = init_count;
    list_init(&sem->wait_list);
}

/**
 * 申请信号量
 */
void sem_wait (sem_t * sem) {
    irq_state_t state = irq_enter_protection();

    if (sem->count > 0) {
        sem->count--;
    } else {
        // 从就绪队列中移除，然后加入信号量的等待队列
        task_t * curr = task_current();
        task_set_block(curr);
        list_insert_last(&sem->wait_list, &curr->wait_node);
        task_dispatch();
    }

    irq_leave_protection(state);
}

/**
 * 释放信号量
 */
void sem_notify (sem_t * sem) {

    irq_state_t state = irq_enter_protection();

    if (list_count(&sem->wait_list)) {
        list_node_t * node = list_remove_first(&sem->wait_list);
        task_t * task = list_node_parent(node, task_t, wait_node);
        task_set_ready(task);

        task_dispatch();
    } else {
        sem->count++;
    }

    irq_leave_protection(state);
}

/**
 * 获取信号量的当前值
 */
int sem_count (sem_t * sem) {

    irq_state_t state = irq_enter_protection();
    int count = sem->count;
    irq_leave_protection(state);

    return count;
}