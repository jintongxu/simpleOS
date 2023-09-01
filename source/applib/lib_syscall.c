/**
 * 系统调用接口
 *
 */
#include "lib_syscall.h"
#include <stdlib.h>
#include <string.h>


/**
 * 执行系统调用
 */
int sys_call (syscall_args_t * args) {
    // 使用特权级0,其实比3高即可，偏移量不需要，填0即可。类似于far_jump函数的实现
	uint32_t addr[] = {0, SELECTOR_SYSCALL | 0};
    int ret;

    // 采用调用门, 这里只支持5个参数
    // 用调用门的好处是会自动将参数复制到内核栈中，这样内核代码很好取参数
    // 而如果采用寄存器传递，取参比较困难，需要先压栈再取
    __asm__ __volatile__(
            "push %[arg3]\n\t"
            "push %[arg2]\n\t"
            "push %[arg1]\n\t"
            "push %[arg0]\n\t"
            "push %[id]\n\t"
            "lcalll *(%[a])"
            :"=a"(ret)
            : [arg3]"r"(args->arg3), [arg2]"r"(args->arg2), [arg1]"r"(args->arg1),
            [arg0]"r"(args->arg0), [id]"r"(args->id),
            [a]"r"(addr));
    return ret;
}

int msleep(int ms) {
    if (ms <= 0) {
        return 0;
    }

    syscall_args_t args;
    args.id = SYS_sleep;
    args.arg0 = ms;

    return sys_call(&args);
}

// 获取进程id
int getpid (void) {
    syscall_args_t args;
    args.id = SYS_getpid;

    return sys_call(&args);
}

// 临时调试用的
void print_msg (const char * fmt, int arg) {
    syscall_args_t args;
    args.id = SYS_printmsg;
    args.arg0 = (int)fmt;
    args.arg1 = arg;

    sys_call(&args);
}


int fork () {
    syscall_args_t args;
    args.id = SYS_fork;

    return sys_call(&args);
}

/* 
    运行另外一个指定的程序。它会把新程序加载到当前进程的内存空间内，
    当前的进程会被丢弃，它的堆、栈和所有的段数据都会被新进程相应的部分代替,
    然后会从新程序的初始化代码和 main 函数开始运行。同时，进程的 ID 将保持不变.
*/
int execve(const char *name, char * const *argv, char * const *env) {
    syscall_args_t args;
    args.id = SYS_execve;
    args.arg0 = (int)name;
    args.arg1 = (int)argv;
    args.arg2 = (int)env;
    return sys_call(&args);
}


int yield (void) {
    syscall_args_t args;
    args.id = SYS_yield;

    return sys_call(&args);
}



int open (const char * name, int flags, ...) {
    syscall_args_t args;
    args.id = SYS_open;
    args.arg0 = (int)name;
    args.arg1 = (int)flags;

    return sys_call(&args);
}

// 文件Id, 读取的位置，读写的长度。
int read (int file, char * ptr, int len) {
    syscall_args_t args;
    args.id = SYS_read;
    args.arg0 = (int)file;
    args.arg1 = (int)ptr;
    args.arg2 = (int)len;
    return sys_call(&args);
}

int write (int file, char * ptr, int len) {
    syscall_args_t args;
    args.id = SYS_write;
    args.arg0 = (int)file;
    args.arg1 = (int)ptr;
    args.arg2 = (int)len;
    return sys_call(&args);
}

int close (int file) {
    syscall_args_t args;
    args.id = SYS_close;
    args.arg0 = (int)file;

    return sys_call(&args);
}

int lseek (int file, int ptr, int dir) {
    syscall_args_t args;
    args.id = SYS_lseek;
    args.arg0 = (int)file;
    args.arg1 = (int)ptr;
    args.arg2 = (int)dir;
    return sys_call(&args);
}


/**
 * 判断文件描述符与tty关联
 */
int isatty (int file) {
    syscall_args_t args;
    args.id = SYS_isatty;
    args.arg0 = (int)file;
  
    return sys_call(&args);
}

/**
 * 获取文件的状态
 */
int fstat (int file, struct stat * st) {
    syscall_args_t args;
    args.id = SYS_fstat;
    args.arg0 = (int)file;
    args.arg1 = (int)st;
  
    return sys_call(&args);
}

void * sbrk (ptrdiff_t incr) {
    syscall_args_t args;
    args.id = SYS_sbrk;
    args.arg0 = (int)incr;
  
    return (void *)sys_call(&args);
}

int dup (int file) {
    syscall_args_t args;
    args.id = SYS_dup;
    args.arg0 = (int)file;
  
    return sys_call(&args);
}

int ioctl (int file, int cmd, int arg0, int arg1) {
    syscall_args_t args;
    args.id = SYS_ioctl;
    args.arg0 = (int)file;
    args.arg1 = (int)cmd;
    args.arg2 = (int)arg0;
    args.arg3 = (int)arg1;
  
    return sys_call(&args);
}


int unlink (const char * pathname) {
    syscall_args_t args;
    args.id = SYS_unlink;
    args.arg0 = (int)pathname;

    return sys_call(&args);
}


void _exit(int status) {
    syscall_args_t args;
    args.id = SYS_exit;
    args.arg0 = (int)status;
  
    sys_call(&args);
}


int wait (int * status) {
    syscall_args_t args;
    args.id = SYS_wait;
    args.arg0 = (int)status;
  
    return sys_call(&args);
}


DIR * opendir (const char * path) {
    DIR * dir = (DIR *)malloc(sizeof(DIR));
    if (dir == (DIR *)0) {
        return (DIR *)0;
    }

    syscall_args_t args;
    args.id = SYS_opendir;
    args.arg0 = (int)path;
    args.arg1 = (int)dir;
    int err = sys_call(&args);
    if (err < 0) {
        free(dir);
        return (DIR *)0;
    }

    return dir;
}

struct dirent * readdir (DIR * dir) {
    syscall_args_t args;
    args.id = SYS_readdir;
    args.arg0 = (int)dir;
    args.arg1 = (int)&dir->dirent;
    int err = sys_call(&args);
    if (err < 0) {
        return (struct dirent *)0;
    }

    return &dir->dirent;
}

int closedir (DIR * dir) {
    syscall_args_t args;
    args.id = SYS_closedir;
    args.arg0 = (int)dir;
    sys_call(&args);

    free(dir);
    return 0;
}