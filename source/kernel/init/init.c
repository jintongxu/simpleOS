#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "os_cfg.h"
#include "dev/time.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "core/task.h"
#include "tools/list.h"
#include "ipc/sem.h"


static boot_info_t * init_boot_info;

void kernel_init (boot_info_t * boot_info) {

    cpu_init();

    log_init();
    irq_init();
    time_init();
    task_manager_init();

}

static task_t first_task;
static uint32_t init_task_stack[1024];
static task_t init_task;
static sem_t sem;

void init_task_entry (void) {
    int count = 0;
    for (;;) {
        sem_wait(&sem);
        
        log_printf("int task %d", count++);
    }
}

void list_test() {
    list_t list;
    list_node_t nodes[5];

    list_init(&list);
    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));
    
    // 头部插入测试
    for (int i = 0; i < 5; i ++) {
        list_node_t * node = nodes + i;

        log_printf("insert first to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_first(&list, node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));

    
    // 尾部插入测试
    list_init(&list);
    for (int i = 0; i < 5; i ++) {
        list_node_t * node = nodes + i;

        log_printf("insert first to last: %d, 0x%x", i, (uint32_t)node);
        list_insert_last(&list, node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));


    // 头部删除测试
    for (int i = 0; i < 5; i ++ ) {
        list_node_t * node = list_remove_first(&list);
        log_printf("remove first from list: %d, 0x%x", i, (uint32_t)node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));
    
    // remove node 删除指定节点
    for (int i = 0; i < 5; i ++) {
        list_node_t * node = nodes + i;

        log_printf("insert first to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_last(&list, node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));

    for (int i = 0; i < 5; i ++ ) {
        list_node_t * node = nodes + i;
        log_printf("remove first from list: %d, 0x%x", i, (uint32_t)node);
        list_remove(&list, node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));
    
    struct type_t {
        int i;
        list_node_t node;
    }v = {0x123456};

    list_node_t * v_node = &v.node;
    struct type_t * p = list_node_parent(v_node, struct type_t, node);
    if (p->i != 0x123456) {
        log_printf("error");
    }


}

void init_main (void) {
    // list_test();

    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    log_printf("%d %d %x %c", 1234, -12345, 0x123456, 'a');

    task_init(&init_task, "init task" ,(uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_first_init();
    // task_init(&first_task, 0, 0);  // 后面两个参数为0：first_task跑起来后已经运行，不需要从tss中加载初始化的值，因此里面的值无所谓，后面切换的时候也会保存状态。
    // write_tr(first_task.tss_sel);  // 对任务寄存器tr进行初始化

    sem_init(&sem, 0);

    irq_enable_global();

    int count = 0;
    for (;;) {
        log_printf("first main %d", count++);
        sem_notify(&sem);

        sys_sleep(1000);
    }

    init_task_entry();

}