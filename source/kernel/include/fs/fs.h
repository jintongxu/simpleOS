#ifndef FS_H
#define FS_H

/*
    文件读写关
    name: 文件路径
    flags: 标志位
    ... : 其他参数
*/
int sys_open(const char * name, int flags, ...);
int sys_read(int file, char * ptr, int len);
int sys_write(int file, char * ptr, int len);
int sys_lseek(int file,  int ptr, int dir);
int sys_close(int file);


#endif