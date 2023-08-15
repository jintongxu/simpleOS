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
#include "dev/console.h"
#include "dev/kbd.h"
#include "fs/fs.h"


static boot_info_t * init_boot_info;


void kernel_init (boot_info_t * boot_info) {

    cpu_init();
    irq_init();
    log_init();
    
    memory_init(boot_info);
    fs_init();

    time_init();
    task_manager_init();

}

void move_to_first_task (void) {
    task_t * curr = task_current();
    ASSERT(curr != 0);

    tss_t * tss = &(curr->tss);
    __asm__ __volatile__(
         // 模拟中断返回，切换入第1个可运行应用进程
        // 不过这里并不直接进入到进程的入口，而是先设置好段寄存器，再跳过去
        "push %[ss]\n\t"			// SS
        "push %[esp]\n\t"			// ESP
        "push %[eflags]\n\t"           // EFLAGS
        "push %[cs]\n\t"			// CS
        "push %[eip]\n\t"		    // ip
        "iret\n\t"::[ss]"r"(tss->ss),  [esp]"r"(tss->esp), [eflags]"r"(tss->eflags),
        [cs]"r"(tss->cs), [eip]"r"(tss->eip));
    
}

void init_main (void) {
    // list_test();

    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    log_printf("%d %d %x %c", 1234, -12345, 0x123456, 'a');

    task_first_init();
    move_to_first_task();

}