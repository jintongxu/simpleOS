#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "os_cfg.h"
#include "dev/time.h"
#include "tools/log.h"


static boot_info_t * init_boot_info;

void kernel_init (boot_info_t * boot_info) {
    init_boot_info = boot_info;
    cpu_init();

    log_init();
    irq_init();
    time_init();
}

void init_main (void) {
    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    log_printf("%d %d %x %c", 1234, -12345, 0x123456, 'a');
    

    int a = 3 / 0;
    // irq_enable_global();
    for (;;) {}
}