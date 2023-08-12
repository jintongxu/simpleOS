#include "lib_syscall.h"
#include "stdio.h"


int main(int argc, char ** argv) {
    sbrk(0);
    sbrk(100);
    sbrk(200);
    sbrk(4096*2 + 200);
    sbrk(4096*5 + 1234);
    
    printf("Hello from shell\n");
    printf("OsTest\n");
    for (int i = 0; i < argc; i++) {
        printf("arg: %s\n", argv[i]);
    }

    fork();
    yield();

    for (;;) {
        printf("shell pid=%d\n", getpid());
        msleep(1000);
    }
}