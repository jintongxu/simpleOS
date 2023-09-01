//
// https://wiki.osdev.org/Programmable_Interval_Timer
//
#include "comm/types.h"
#include "dev/time.h"
#include "cpu/irq.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"

static uint32_t sys_tick;


/**
 * 定时器中断处理函数
 */
void do_handler_time (exception_frame_t * frame) {
    sys_tick++;

    // 先发EOI，而不是放在最后
    // 放最后将从任务中切换出去之后，除非任务再切换回来才能继续噢应
    pic_send_eoi(IRQ0_TIMER);    // 告诉系统可以进行下一个进程了
    task_time_tick();       // 做有关任务定时方面的处理

}


/**
 * 初始化硬件定时器
 */
static void init_pit (void) {

    uint32_t reload_count =  PIT_OSC_FREQ * OS_TICKS_MS / 1000;
    outb(PIT_COMMAND_MODE_PORT, PIT_CHANNEL | PIT_LOAD_LOHI | PIT_MODE3);
    outb(PIT_CHANNEL0_DATA_PORT, reload_count & 0xFF);              // 加载低8位
    outb(PIT_CHANNEL0_DATA_PORT, (reload_count >> 8) & 0xFF);       // 再加载高8位

    irq_install(IRQ0_TIMER, (irq_handler_t)exception_handler_time);
    irq_enable(IRQ0_TIMER);
}


/**
 * 定时器初始化
 */
void time_init (void) {
    sys_tick = 0;
    init_pit();
}
