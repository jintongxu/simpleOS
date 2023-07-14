#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "os_cfg.h"
#include "dev/time.h"


static boot_info_t * init_boot_info;

void kernel_init (boot_info_t * boot_info) {
    init_boot_info = boot_info;
    cpu_init();
    irq_init();
    time_init();
}

void init_main (void) {
    // int a = 3 / 0;
    // irq_enable_global();
    for (;;) {}
}