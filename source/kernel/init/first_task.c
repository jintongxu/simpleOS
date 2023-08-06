#include "core/task.h"
#include "tools/log.h"
#include "applib/lib_syscall.h"

int first_task_main (void) {
    int pid = getpid();
    
    for(;;) {
        print_msg("task id=%d", pid);
        // sys_sleep(1000);
        msleep(1000);
    }


    return 0;
}