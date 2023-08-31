#include "lib_syscall.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "main.h"
#include <getopt.h>
#include <sys/file.h>
#include "fs/file.h"
#include "dev/tty.h"


static cli_t cli;
static const char * promot = "sh >>";

// help 输出帮助信息
static int do_help (int argc, char **argv) {   
    const cli_cmd_t * start = cli.cmd_start;
    while(start < cli.cmd_end) {
        printf("%s %s\n", start->name, start->usage);
        start++;
    }
    return 0;
}

static int do_clear (int argc, char **argv) {
    printf("%s", ESC_CLEAR_SCREEN);
    printf("%s", ESC_MOVE_CURSOR(0, 0));    // 移动光标位置 里面是0行0列
    return 0;
}

// 回显操作
static int do_echo (int argc, char **argv) {
     // 只有一个参数，需要先手动输入，再输出
    if (argc == 1) {
        char msg_buf[128];

        fgets(msg_buf, sizeof(msg_buf), stdin);
        msg_buf[sizeof(msg_buf) - 1] = '\0';
        puts(msg_buf);
        return 0;
    }

    int count = 1;
    int ch;
    while ((ch = getopt(argc, argv, "n:h")) != -1) {
        switch (ch)
        {
        case 'h':
            // 有参数 -h
            puts("echo any message");
            puts("Usage: echo [-n count] message");
            optind = 1;  
            return 0;
        case 'n':
            // 有 -n num
            count = atoi(optarg);
            break;
        case '?':
            if (optarg) {
                fprintf(stderr, "Unknown option: -%s\n", optarg);
            }
            optind = 1;  
            return -1;
        default:
            break;
        }
    }

    if (optind > argc - 1) {
        fprintf(stderr, "Message is empty\n");
        optind = 1;  
        return -1;
    }

    char * msg = argv[optind];
    for (int i = 0; i < count; i++) {
        puts(msg);
    }
    optind = 1;  
    return 0;

}

static int do_exit (int argc, char **argv) {
    exit(0);
    return 0;
}

static int do_ls (int argc, char **argv) {
    DIR * p_dir = opendir("temp");
    if (p_dir == NULL) {
        printf("open dir failed.");
        return -1;
    }

    struct dirent * entry;
    while((entry = readdir(p_dir)) != NULL) {
        strlwr(entry->name);
        printf("%c %s %d\n",
            entry->type == FILE_DIR ? 'd' : 'f',
            entry->name,
            entry->size);
    }
    closedir(p_dir);
    return 0;
}


static int do_less (int argc, char **argv) {
    int line_mode = 0;      // 是否以行查看的标志



    int ch;
    while ((ch = getopt(argc, argv, "lh")) != -1) {
        switch (ch)
        {
        case 'h':
            // 有参数 -h
            puts("show file content");
            puts("Usage: less [-l] file");
            optind = 1;  
            return 0;
        case 'l':
            line_mode = 1;
            break;
        case '?':
            if (optarg) {
                fprintf(stderr, "Unknown option: -%s\n", optarg);
            }
            optind = 1;  
            return -1;
        default:
            break;
        }
    }

    if (optind > argc - 1) {
        fprintf(stderr, "no file\n");
        optind = 1;  
        return -1;
    }


    FILE * file = fopen(argv[optind], "r");
    if (file == NULL) {
        fprintf(stderr, "open file failed. %s", argv[optind]);
        optind = 1;
        return -1;
    }

    char * buf = (char *)malloc(255);
    // 如果一次性打印所有文件内容
    if (line_mode == 0) {
        while (fgets(buf, 255, file) != NULL) {
            fputs(buf, stdout);
        }
    } else {
        // 不使用缓存，这样能直接立即读取到输入而不用等回车
        setvbuf(stdin, NULL, _IONBF, 0);
        ioctl(0, TTY_CMD_ECHO, 0, 0);

        while (1) {
            char * b = fgets(buf, 255, file);
            if (b == NULL) {
                break;
            }

            fputs(buf, stdout);

            int ch;
            while ((ch = fgetc(stdin)) != 'n') {
                if (ch == 'q') {
                    goto less_quit;
                }
            }
        }
less_quit:
        // 恢复为行缓存
        setvbuf(stdin, NULL, _IOLBF, BUFSIZ);
        ioctl(0, TTY_CMD_ECHO, 1, 0);
    }
    
    free(buf);

    fclose(file);
    optind = 1;  
    return 0;
}


static int do_cp (int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "no [from] or no [to]");
        return -1;
    }

    FILE * from, * to;
    from = fopen(argv[1], "rb");        // 只读和二进制打开文件
    to = fopen(argv[2], "wb");
    if (!from || !to) {
        fprintf(stderr, "open file failed");
        goto ls_failed;
    }

    char * buf = (char *)malloc(255);
    int size;
    while((size = fread(buf, 1, 255, from)) > 0) {
        fwrite(buf, 1, 255, to);
    }
    free(buf);


ls_failed:
    if (from) {
        fclose(from);
    }

    if (to) {
        fclose(to);
    }
    return 0;
}



static int do_rm (int argc, char ** argv) {
    if (argc < 2) {
        fprintf(stderr, "no file");
        return -1;
    }


    int err = unlink(argv[1]);
    if (err < 0) {
        fprintf(stderr, "rm file failed: %s", argv[1]);
        return err;
    }


    return 0;
}



static const cli_cmd_t cmd_list[] = {
    {
        .name = "help",
        .usage = "help -- list supported command",
        .do_func = do_help,
    },
    {
        .name = "clear",
        .usage = "clear -- clear screen",
        .do_func = do_clear,
    },
    {
        .name = "echo",
        .usage = "echo [-n count] msg -- echo something",
        .do_func = do_echo,
    },
    {
        .name = "ls",
        .usage = "ls -- list director",
        .do_func = do_ls,
    },
    {
        .name = "less",
        .usage = "less [-l] file -- show file",
        .do_func = do_less,
    },
    {
        .name = "cp",
        .usage = "cp src dest",
        .do_func = do_cp,
    },
    {
        .name = "rm",
        .usage = "rm file - remove file",
        .do_func = do_rm,
    },
    {
        .name = "quit",
        .usage = "quit from shell",
        .do_func = do_exit,
    },
};

// 显示命令行提示符
static void show_promot (void) {
    printf("%s", cli.promot);
    fflush(stdout);
}

// 在内部命令中搜索
static const cli_cmd_t * find_builtin (const char * name) {
    for (const cli_cmd_t * cmd = cli.cmd_start; cmd < cli.cmd_end; cmd++) {
        if (strcmp(cmd->name, name) != 0) {
            continue;
        }

        return cmd;
    }

    return (const cli_cmd_t*)0;
}

// 运行内部命令
static void run_builtin (const cli_cmd_t * cmd, int argc, char ** argv) {
    int ret = cmd->do_func(argc, argv);
    if (ret < 0) {
        fprintf(stderr, ESC_COLOR_ERROR"error: %d\n"ESC_COLOR_DEFAULT, ret);
    }
}

static void cli_init (const char * promot, const cli_cmd_t * cmd_list, int size) {
    cli.promot = promot;
    memset(cli.curr_input, 0, CLI_INPUT_SIZE);
    cli.cmd_start = cmd_list;
    cli.cmd_end = cmd_list + size;
}


/**
 * 遍历搜索目录，看看文件是否存在，存在返回文件所在路径
 */
static const char * find_exec_path (const char * file_name) {
    static char path[255];

    int fd = open(file_name, 0);
    if (fd < 0) {
        sprintf(path, "%s.elf", file_name);
        fd = open(path, 0);
        if (fd < 0) {
            return (const char * )0;
        }
        close(fd);
        return path;
    } else {
        close(fd);
        return file_name;
    }
}

// 试图运行当前文件
static void run_exec_file(const char * path, int argc, char **argv) {
    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed %s", path);
    } else if (pid == 0) {
        int err = execve(path, argv, (char * const *)0);
        if (err < 0) {
            fprintf(stderr, "exec failed: %s", path);
        }
        exit(-1);
    } else {
        // 在父进程中
        int status;
        int pid = wait(&status);
        fprintf(stderr, "cmd %s result: %d, pid = %d\n", path, status, pid);
    }
}


int main(int argc, char ** argv) {
    open(argv[0], O_RDWR);           // int fd = 0   stdin 三个都是指向同一个tty设备，tty0，只是在进程的 文件表中，fd不一样。  => tty0
    dup(0);                     // int fd = 1   stdout  => tty0
    dup(0);                     // int fd = 2   stdeer  => tty0

    
    cli_init(promot, cmd_list, sizeof(cmd_list) / sizeof(cmd_list[0]));
    for (;;) {
        show_promot();
        char * str = fgets(cli.curr_input, CLI_INPUT_SIZE, stdin);
        if (!str) {
            continue;
        }


        // 将从屏幕输入的字符串最后的 \r\n 去掉
        char * cr = strchr(cli.curr_input, '\n');       // 查找回车所在的位置
        if (cr) {
            // 把 \n 更换 为 \0
            *cr = '\0';
        }
        cr = strchr(cli.curr_input, '\r');       // 查找回车所在的位置
        if (cr) {
            // 把 \r 更换 为 \0
            *cr = '\0';
        }

        
        int argc = 0;
        char * argv[CLI_MAX_ARG_COUNT];
        memset(argv, 0, sizeof(argv));

        // 将输入的命令和参数，按空格分割开
        const char * space = " ";
        char * token = strtok(cli.curr_input, space);
        while(token) {
            argv[argc++] = token;
            token = strtok(NULL, space);
        }

        if (argc == 0) {
            continue;
        }

        const cli_cmd_t * cmd = find_builtin(argv[0]);      // 看看是什么命令
        if (cmd) {
            run_builtin(cmd, argc, argv);
            continue;
        }

        const char * path = find_exec_path(argv[0]);        // 看看要运行的文件存不存在
        if (path) {
            // 如果不是系统调用，试图运行当前文件
            run_exec_file(path, argc, argv);
            continue;
        }

        

        // exec
        fprintf(stderr, ESC_COLOR_ERROR"Unknown command: %s\n"ESC_COLOR_DEFAULT, cli.curr_input);
    }
}