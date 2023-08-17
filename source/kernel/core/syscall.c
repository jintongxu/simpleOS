#include "core/syscall.h"
#include "core/task.h"
#include "tools/log.h"
#include "fs/fs.h"

typedef int (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

void sys_print_msg (char * fmt, int arg) {
    log_printf(fmt, arg);
}

static const syscall_handler_t sys_table[] = {
	[SYS_sleep] = (syscall_handler_t)sys_sleep,
    [SYS_getpid] = (syscall_handler_t)sys_getpid,
    [SYS_fork] = (syscall_handler_t)sys_fork,
    [SYS_printmsg] = (syscall_handler_t)sys_print_msg,
    [SYS_execve] = (syscall_handler_t)sys_execve,
	[SYS_yield] = (syscall_handler_t)sys_sched_yield,
	[SYS_exit] = (syscall_handler_t)sys_exit,

	[SYS_open] = (syscall_handler_t)sys_open,
	[SYS_read] = (syscall_handler_t)sys_read,
	[SYS_write] = (syscall_handler_t)sys_write,
	[SYS_close] = (syscall_handler_t)sys_close,
	[SYS_lseek] = (syscall_handler_t)sys_lseek,

	[SYS_isatty] = (syscall_handler_t)sys_isatty,
	[SYS_sbrk] = (syscall_handler_t)sys_sbrk,
	[SYS_fstat] = (syscall_handler_t)sys_fstat,
	[SYS_dup] = (syscall_handler_t)sys_dup,
	
};

void do_handler_syscall (syscall_frame_t * frame) {
	// 超出边界，返回错误
    if (frame->func_id < sizeof(sys_table) / sizeof(sys_table[0])) {
		// 查表取得处理函数，然后调用处理
		syscall_handler_t handler = sys_table[frame->func_id];
		if (handler) {
			int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
			frame->eax = ret;  // 设置系统调用的返回值，由eax传递
            return;
		}
	}

	// 不支持的系统调用，打印出错信息
	task_t * task = task_current();
	log_printf("task: %s, Unknown syscall: %d", task->name,  frame->func_id);
    frame->eax = -1;  // 设置系统调用的返回值，由eax传递
}