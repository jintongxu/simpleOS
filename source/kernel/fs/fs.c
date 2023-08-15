#include "fs/fs.h"
#include "tools/klib.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "sys/stat.h"
#include "dev/console.h"
#include "tools/log.h"
#include "fs/file.h"
#include "dev/dev.h"
#include "core/task.h"


static uint8_t TEMP_ADDR[100*1024];
static uint8_t * temp_pos;   // 当前位置指针
#define TEMP_FILE_ID    100

static void read_disk(int sector, int sector_count, uint8_t * buf) {
    outb(0x1F6, (uint8_t) (0xE0));

	outb(0x1F2, (uint8_t) (sector_count >> 8));
    outb(0x1F3, (uint8_t) (sector >> 24));		// LBA参数的24~31位
    outb(0x1F4, (uint8_t) (0));					// LBA参数的32~39位
    outb(0x1F5, (uint8_t) (0));					// LBA参数的40~47位

    outb(0x1F2, (uint8_t) (sector_count));
	outb(0x1F3, (uint8_t) (sector));			// LBA参数的0~7位
	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA参数的16~23位

	outb(0x1F7, (uint8_t) 0x24);

	// 读取数据
	uint16_t *data_buf = (uint16_t*) buf;
	while (sector_count-- > 0) {
		// 每次扇区读之前都要检查，等待数据就绪
		while ((inb(0x1F7) & 0x88) != 0x8) {}

		// 读取并将数据写入到缓存中
		for (int i = 0; i < SECTOR_SIZE / 2; i++) {
			*data_buf++ = inw(0x1F0);
		}
	}
}


static int is_path_valid (const char * path) {
    if ((path == (const char *)0) || (path[0] == '\0')) {
        return 0;
    }

    return 1;
}

// 文件打开
int sys_open(const char * name, int flags, ...) {
    if (kernel_strncmp(name, "tty", 3) == 0) {
        // 如果是以 tty 开头的话
        if (!is_path_valid(name)) {
            log_printf("path is not valid");
            return -1;
        }

        // 分配文件描述符链接。这个过程中可能会被释放
        int fd = -1;
        file_t * file = file_alloc();
        if (file) {
            fd = task_alloc_fd(file);
            if (fd < 0) {
                goto sys_open_failed;
            }
        } else {
            goto sys_open_failed;
        }

        if (kernel_strlen(name) < 5) {
            goto sys_open_failed;
        }
        int num = name[4] - '0';
        int dev_id = dev_open(DEV_TTY, num, 0);
        if (dev_id < 0) {
            goto sys_open_failed;
        }

        file->dev_id = dev_id;
        file->mode = 0;
        file->pos = 0;
        file->ref = 1;
        file->type = FILE_TTY;
        kernel_strncpy(file->file_name, name, FILE_NAME_SIZE);
        return fd;

sys_open_failed:
        if (file) {
            file_free(file);
        }

        if (fd >= 0) {
            task_remove_fd(fd);
        }
        return -1;
    } else {
        if (name[0] == '/') {
        read_disk(5000, 80, (uint8_t *)TEMP_ADDR);   // 因为将shell放在了磁盘5000的位置
        temp_pos = (uint8_t *)TEMP_ADDR;
        return TEMP_FILE_ID;
        }
    }


    return -1;
}

// 文件读取
int sys_read(int file, char * ptr, int len) {
    if (file == TEMP_FILE_ID) {
        kernel_memcpy(ptr, temp_pos, len);
        temp_pos += len;
        return len;
    } else {
        file = 0;

        file_t * p_file = task_file(file);
        if (!p_file) {
            log_printf("file not opened");
            return -1;
        }
        return dev_read(p_file->dev_id, 0, ptr, len);
    }

    return -1;
}

int sys_write(int file, char * ptr, int len) {
    file_t * p_file = task_file(file);
    if (!p_file) {
        log_printf("file not opened");
        return -1;
    }

    return dev_write(p_file->dev_id, 0, ptr, len);
}

// 移动读写的指针
int sys_lseek(int file, int ptr, int dir) {
    if (file == TEMP_FILE_ID) {
        temp_pos = (uint8_t *)(TEMP_ADDR + ptr);
        return 0;
    }

    return -1;
}

int sys_close(int file) {
    return 0;
}

int sys_isatty(int file) {
    return -1;
}


int sys_fstat(int file, struct stat * st) {
    return -1;
}


// 复制一个文件描述符
int sys_dup (int file) {
    // 超出进程所能打开的全部，退出
    if ((file < 0) && (file >= TASK_OFILE_NR)) {
        // 如果 id 无效
        log_printf("file %d is not valid.", file);
        return -1;
    }

    file_t * p_file = task_file(file);
    if (!p_file) {
        log_printf("file not open");
        return -1;
    }

    int fd = task_alloc_fd(p_file);     // 新fd指向同一描述符
    if (fd > 0) {
        p_file->ref++;      // 增加引用
        return fd;
    }

    log_printf("No task file avaliable");
    return -1;

}


// 文件系统的初始化操作
void fs_init (void) {
    file_table_init();
}