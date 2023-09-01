/**
 * os配置
 */
#ifndef OS_CFG_H
#define OS_CFG_H


#define GDT_TABLE_SIZE          256                 // GDT表项数量

#define KERNEL_SELECTOR_CS      (1 * 8)             // 内核代码段描述符
#define KERNEL_SELECTOR_DS      (2 * 8)             // 内核数据段描述符
#define SELECTOR_SYSCALL        (3 * 8)             // 调用门的选择子

#define KERNEL_STACK_SIZE       (8 * 1024)          // 内核栈


#define OS_TICKS_MS     10                          // 每毫秒的时钟数
#define IDLE_TASK_SIZE  1024                        // 空闲任务栈

#define OS_VERSION      "1.0.0"                     // OS版本号                        

#define TASK_NR         128                         // 进程的数量

#define ROOT_DEV        DEV_DISK, 0xb1              // 根目录所在的设备

#endif