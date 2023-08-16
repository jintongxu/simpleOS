#include "lib_syscall.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "main.h"


static cli_t cli;
static const char * promot = "sh >>";

static int do_help (int argc, char **argv) {
    return 0;
}

static const cli_cmd_t cmd_list[] = {
    {
        .name = "help",
        .usage = "help -- list supported command",
        .do_func = do_help,
    },
};

// 显示命令行提示符
static void show_promot (void) {
    printf("%s", cli.promot);
    fflush(stdout);
}

static void cli_init (const char * promot, const cli_cmd_t * cmd_list, int size) {
    cli.promot = promot;
    memset(cli.curr_input, 0, CLI_INPUT_SIZE);
    cli.cmd_start = cmd_list;
    cli.cmd_end = cmd_list + size;
}

int main(int argc, char ** argv) {
    open(argv[0], 0);           // int fd = 0   stdin 三个都是指向同一个tty设备，tty0，只是在进程的 文件表中，fd不一样。  => tty0
    dup(0);                     // int fd = 1   stdout  => tty0
    dup(0);                     // int fd = 2   stdeer  => tty0

    
    cli_init(promot, cmd_list, sizeof(cmd_list) / sizeof(cmd_list[0]));
    for (;;) {
        show_promot();
        gets(cli.curr_input);
    }
}