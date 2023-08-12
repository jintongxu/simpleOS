#include "lib_syscall.h"
#include "stdio.h"


int main(int argc, char ** argv) {
    sbrk(0);
    sbrk(100);
    sbrk(200);
    sbrk(4096*2 + 200);
    sbrk(4096*5 + 1234);

    printf("abef\b\b\b\bcd\n");     // cdef  \b 是光标左移1位，
    printf("abcd\x7f;fg\n");        // abc;fg
    
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