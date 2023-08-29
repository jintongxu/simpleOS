#ifndef FS_H
#define FS_H

#include "file.h"
#include "tools/list.h"
#include "ipc/mutex.h"
#include "fatfs/fatfs.h"
#include "applib/lib_syscall.h"

/*
    文件读写关
    name: 文件路径
    flags: 标志位
    ... : 其他参数
*/
struct stat;

struct _fs_t;

// 文件系统操作接口
typedef struct _fs_op_t {
    int (*mount)(struct _fs_t * fs, int major, int minor);      // 挂载文件系统
    void (*unmount)(struct _fs_t * fs);      // 取消文件系统挂载
    int (*open) (struct _fs_t * fs, const char * path, file_t * file);
    int (*read) (char * buf, int size, file_t * file);
    int (*write) (char * buf, int size, file_t * file);
    void (*close) (file_t * file);
    int (*seek) (file_t * file, uint32_t offset, int dir);
    int (*stat) (file_t * file, struct stat *st);
}fs_op_t;

#define FS_MOUNTP_SIZE      512

typedef enum _fs_type_t {
    FS_FAT16,
    FS_DEVFS,
}fs_type_t;

typedef struct _fs_t {
    char mount_point[FS_MOUNTP_SIZE];       // 挂载点路径长
    fs_type_t type;

    fs_op_t * op;           // 文件系统操作接口
    void * data;            // 文件系统的操作数据
    int dev_id;             // 所属的设备
    list_node_t node;       // 下一结点
    mutex_t * mutex;        // 文件系统操作互斥信号量

    union
    {
        fat_t fat_data;
    };

}fs_t;

void fs_init (void);    // 对文件系统的初始化


int path_to_num(const char * path, int * num);
const char * path_next_child (const char * path);

int sys_open(const char * name, int flags, ...);
int sys_read(int file, char * ptr, int len);
int sys_write(int file, char * ptr, int len);
int sys_lseek(int file,  int ptr, int dir);
int sys_close(int file);
int sys_isatty(int file);
int sys_fstat(int file, struct stat * st);
char * sys_sbrk(int incr);

int sys_dup (int file);

int sys_opendir (const char * name, DIR * dir);
int sys_readdir (DIR * dir, struct dirent * dirent);
int sys_closedir (DIR * dir);

#endif