/**
 * 进程启动C部分代码
 *
 */
#include  <stdint.h>
#include  "lib_syscall.h"
#include  <stdlib.h>

int main(int argc, char ** argv);

extern uint8_t  __bss_start__[], __bss_end__[];


/**
 * @brief 应用的初始化，C部分
 */
void cstart (int argc, char **argv) {   
    // 完成 bss区域 的清零
    uint8_t * start = __bss_start__;
    while(start < __bss_end__) {
        *start++;
    }

    exit(main(argc, argv));
}