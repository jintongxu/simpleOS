#include "core/task.h"
#include "tools/log.h"
#include "applib/lib_syscall.h"
#include "stddef.h"
#include "dev/tty.h"

int first_task_main (void) {
#if 0
    int pid = getpid();
    
    int count = 3;

    print_msg("first task id=%d\n", pid);

    pid = fork();
    if (pid < 0) {
        print_msg("create child proc failed.\n", 0);
    } else if (pid == 0) {
        print_msg("child: %d\n", count);

        char * argv[] = {"arg0", "arg1", "arg2", "arg3", NULL};
        execve("/shell.elf", argv, (char **)0);
    } else {
        print_msg("child task id=%d\n", pid);
        print_msg("parent: %d\n", count);
    }
#endif
    // 创建 8 个进程运行 shell，将 tty 设备号传给shell，
    // 解决其他屏幕不回显问题
    for (int i = 0; i < 1; i++) {
        int pid = fork();
        if (pid < 0) {
            print_msg("create shell failed.", 0);
            break;
        } else if (pid == 0) {
            char tty_num[] = "/dev/tty?";
            tty_num[sizeof(tty_num) - 2] = i + '0';
            char * argv[] = {tty_num, (char *)0, NULL};
            execve("/shell.elf", argv, (char **)0);
            while(1) {
                msleep(1000);
            }
        }

    }


    for(;;) {
        // print_msg("task id=%d", pid);
        // sys_sleep(1000);
        int status;
        wait(&status);


    }


    return 0;
}