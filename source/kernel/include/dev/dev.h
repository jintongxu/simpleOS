#ifndef DEV_H
#define DEV_H

#define DEV_NAME_SIZE      32

enum {
    DEV_UNKNOWN = 0,
    DEV_TTY,
};

// 设备驱动接口
struct _dev_desc_t;
typedef struct _device_t {
    struct _dev_desc_t * desc;  // 设备特性描述符
    
    int mode;       // 操作模式
    int minor;      // 次设备号
    void * data;     // 设备参数
    int open_count;   // 打开次数

}device_t;

// 设备描述结构
typedef struct _dev_desc_t {
    char name[DEV_NAME_SIZE];   // 设备名称
    int major;                  // 主设备号

    int (*open)(device_t * dev);
    int (*read)(device_t * dev, int addr, char * buf, int size);
    int (*write)(device_t * dev, int addr, char * buf, int size);
    int (*control)(device_t * dev, int cmd, int arg0, int arg1);
    void (*close)(device_t * dev);
}dev_desc_t;


int dev_open (int major, int minor, void * data);            // 打开指定的设备
int dev_read (int dev_id, int addr, char * buf, int size);      // 读取指定字节的数据
int dev_write (int dev_id, int addr, char * buf, int size);     // 写指定字节的数据
int dev_control (int dev_id, int cmd, int arg0, int arg1);      // 发送控制命令
void dev_close (int dev_id);            // 关闭设备


#endif