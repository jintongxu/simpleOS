#include "dev/tty.h"
#include "dev/dev.h"

static tty_t  tty_devs[TTY_NR];

int tty_open (device_t * dev) {
    return 0;
}

int tty_read (device_t * dev, int addr, char * buf, int size) {
    return size;
}

int tty_write (device_t * dev, int addr, char * buf, int size) {
    return size;
}

int tty_control (device_t * dev, int cmd, int arg0, int arg1) {
    return 0;
}

void tty_close (device_t * dev) {

}


// 设备描述表: 描述一个设备所具备的特性
dev_desc_t dev_tty_desc = {
    .name = "tty",
    .major = DEV_TTY,
    .open = tty_open,
    .read = tty_read,
    .write = tty_write,
    .control = tty_control,
    .close = tty_close,
};