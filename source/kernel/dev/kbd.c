#include "dev/kbd.h"
#include "cpu/irq.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"


// 键盘初始化
void kbd_init (void) {
    irq_install(IRQ1_KEYBOARD, (irq_handler_t)exception_handler_kbd);
    irq_enable(IRQ1_KEYBOARD);
}

// 按键中断处理程序
void do_handler_kbd (exception_frame_t * frame) {
    uint32_t status = inb(KBD_PORT_STAT);
    if (!(status & KBD_STAT_RECV_READY)) {
        pic_send_eoi(IRQ1_KEYBOARD);
        return;
    }

    uint8_t raw_code = inb(KBD_PORT_DATA);
    log_printf("key: %d", raw_code);

    pic_send_eoi(IRQ0_TIMER);


}
