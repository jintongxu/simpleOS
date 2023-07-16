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


static boot_info_t * init_boot_info;

void kernel_init (boot_info_t * boot_info) {

    cpu_init();

    log_init();
    irq_init();
    time_init();
}

static task_t first_task;
static uint32_t init_task_stack[1024];
static task_t init_task;

void init_task_entry (void) {
    int count = 0;
    for (;;) {
        log_printf("int task %d", count++);
        task_switch_from_to(&init_task, &first_task);
    }
}

void list_test() {
    list_t list;

    list_init(&list);
    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));
    
}

void init_main (void) {
    list_test();

    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    log_printf("%d %d %x %c", 1234, -12345, 0x123456, 'a');

    task_init(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_init(&first_task, 0, 0);  // 后面两个参数为0：first_task跑起来后已经运行，不需要从tss中加载初始化的值，因此里面的值无所谓，后面切换的时候也会保存状态。
    write_tr(first_task.tss_sel);  // 对任务寄存器tr进行初始化

    int count = 0;
    for (;;) {
        log_printf("int main %d", count++);
        task_switch_from_to(&first_task, &init_task);
    }

    init_task_entry();

}