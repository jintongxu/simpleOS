/**
 * 系统调用接口
 *
 */
#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "os_cfg.h"
#include "core/syscall.h"
#include "sys/stat.h"

typedef struct _syscall_args_t {
    int id;
    int arg0;
    int arg1;
    int arg2;
    int arg3;
}syscall_args_t;


int msleep(int ms);
// 获取进程id
int getpid (void);
// 临时调试用的
void print_msg (const char * fmt, int arg);
int fork ();
/* 
    运行另外一个指定的程序。它会把新程序加载到当前进程的内存空间内，
    当前的进程会被丢弃，它的堆、栈和所有的段数据都会被新进程相应的部分代替,
    然后会从新程序的初始化代码和 main 函数开始运行。同时，进程的 ID 将保持不变.
*/
int execve(const char *name, char * const *argv, char * const *env);
int yield (void);


int open (const char * name, int flags, ...);
int read (int file, char * ptr, int len);   // 文件Id, 读取的位置，读写的长度。
int write (int file, char * ptr, int len);
int close (int file);
int lseek (int file, int ptr, int dir);
int ioctl (int file, int cmd, int arg0, int arg1);


int unlink (const char * pathname);

int isatty (int file);
int fstat (int file, struct stat * st);
void * sbrk (ptrdiff_t incr);

int dup (int file);

void _exit(int status);
int wait (int * status);


struct dirent {
    int index;      // 在目录中的偏移
    int type;       // 文件或目录的类型
    char name[255];     // 目录或目录的名称
    int size;           // 文件大小
};

typedef struct _DIR {
    int index;       // 当前遍历的索引
    struct dirent dirent;
}DIR;


DIR * opendir (const char * path);
struct dirent * readdir (DIR * dir);
int closedir (DIR * dir);

#endif