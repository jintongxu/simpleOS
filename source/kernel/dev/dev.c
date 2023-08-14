#include "dev/dev.h"

extern dev_desc_t dev_tty_desc;

// 打开指定的设备
int dev_open (int major, int minor, void * data) {
    return -1;
}


// 读取指定字节的数据
int dev_read (int dev_id, int addr, char * buf, int size) {
    return size;
}

// 写指定字节的数据
int dev_write (int dev_id, int addr, char * buf, int size) {
    return size;
}

// 发送控制命令
int dev_control (int dev_id, int cmd, int arg0, int arg1) {

}

// 关闭设备
void dev_close (int dev_id) {

}


