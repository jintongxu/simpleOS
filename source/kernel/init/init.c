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
#include "core/memory.h"


static boot_info_t * init_boot_info;


void kernel_init (boot_info_t * boot_info) {

    cpu_init();
    log_init();

    
    memory_init(boot_info);


    irq_init();
    time_init();
    task_manager_init();

}

void move_to_first_task (void) {
    task_t * curr = task_current();
    ASSERT(curr != 0);

    tss_t * tss = &(curr->tss);
    __asm__ __volatile__(
        "jmp *%[ip]"::[ip]"r"(tss->eip)
    );
}

void init_main (void) {
    // list_test();

    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    log_printf("%d %d %x %c", 1234, -12345, 0x123456, 'a');

    task_first_init();
    move_to_first_task();

}