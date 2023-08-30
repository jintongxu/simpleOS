#include "dev/tty.h"
#include "dev/dev.h"
#include "tools/log.h"
#include "dev/kbd.h"
#include "dev/console.h"
#include "cpu/irq.h"

static tty_t  tty_devs[TTY_NR];
static int curr_tty = 0;

static tty_t * get_tty (device_t * dev) {
    int tty = dev->minor;
    if ((tty < 0) || (tty >= TTY_NR) || (!dev->open_count)) {
        log_printf("tty is not opened. tty=%d", tty);
        return (tty_t *)0;
    } 

    return tty_devs + tty;
}


// FIFO初始化
void tty_fifo_init (tty_fifo_t * fifo, char * buf, int size) {
    fifo->buf = buf;
    fifo->count = 0;
    fifo->size = size;
    fifo->read = fifo->write = 0;
}

// 往buf中写一字节数据
int tty_fifo_put (tty_fifo_t *fifo, char c) {
    irq_state_t state = irq_enter_protection();
    if (fifo->count >= fifo->size) {
        irq_leave_protection(state);
        return -1;
    }
    
    
    fifo->buf[fifo->write++] = c;
    if (fifo->write >= fifo->size) {
        // 如果超过表的长度就从头开始，因为是循环队列
        fifo->write = 0;
    }
    fifo->count++;
    irq_leave_protection(state);

    return 0;

}

// 从buf中读取
int tty_fifo_get (tty_fifo_t *fifo, char * c) {
    irq_state_t state = irq_enter_protection();
    if (fifo->count <= 0) {
        irq_leave_protection(state);
        return -1;
    }

    
    *c = fifo->buf[fifo->read++];
    if (fifo->read >= fifo->size) {
        // 如果超过表的长度就从头开始，因为是循环队列
        fifo->read = 0;
    }
    fifo->count--;  // 因为取出一个数据就要减1
    irq_leave_protection(state);
    return 0;
}

int tty_open (device_t * dev) {
    int idx = dev->minor;
    if ((idx < 0) || (idx >= TTY_NR)) {
        log_printf("open tty failed. incorrect tty num = %d", idx);
        return -1;
    }

    tty_t * tty = tty_devs + idx;
    tty_fifo_init (&tty->ofifo, tty->obuf, TTY_OBUF_SIZE);
    sem_init(&tty->osem, TTY_OBUF_SIZE);
    tty_fifo_init (&tty->ififo, tty->ibuf, TTY_IBUF_SIZE);
    sem_init(&tty->isem, 0);
    tty->iflags = TTY_INCLR | TTY_IECHO;
    tty->oflags = TTY_OCRLF;
    tty->console_idx = idx;

    kbd_init();
    console_init (idx);


    return 0;
}

int tty_write (device_t * dev, int addr, char * buf, int size) {
    if (size < 0) {
        return -1;
    }

    tty_t * tty = get_tty(dev);
    if (!tty) {
        return -1;
    }

    int len = 0;
    while(size) {
        char c = *buf++;

        if ((c == '\n') && (tty->oflags & TTY_OCRLF)) {
            // 将回车转换为换行加回车，在汇编中 \n 是当前位置的下一行，并不是下一行的开头。
            sem_wait(&tty->osem);
            int err = tty_fifo_put(&tty->ofifo, '\r');
            if (err < 0) {
                break;
            }
        }

        sem_wait(&tty->osem);
        int err = tty_fifo_put(&tty->ofifo, c);
        if (err < 0) {
            break;
        }

        len++;
        size--;

        console_write(tty);

    }

    return len;
}

int tty_read (device_t * dev, int addr, char * buf, int size) {
    if (size < 0) {
        return -1;
    }

    tty_t * tty = get_tty(dev);
    char * pbuf = buf;
    int len = 0;

    // 不断读取，直到遇到文件结束符或者行结束符
    while(len < size) {
        // 等待可用的数据
        sem_wait(&tty->isem);

        // 取出数据
        char ch;
        tty_fifo_get(&tty->ififo, &ch);
        switch (ch)
        {
        case 0x7F:
            // 判断是不是退格键
            // 防止在按下退格键后被终端读取在命令里
            if (len == 0) {
                // 如果缓冲区没有数据
                continue;
            }

            len--;
            pbuf--;
            break;
        case '\n':
            if ((tty->iflags & TTY_INCLR) && (len < size - 1)) {
                *pbuf++ = '\r';
                len++;
            }
            *pbuf++= '\n';
            len++;
            break;
        default:
            *pbuf++ = ch;
            len++;
            break;
        }


        // 设置回显--输入后实时显示在屏幕上
        if (tty->iflags & TTY_IECHO) {
            tty_write(dev, 0, &ch, 1);
        }

        if ((ch == '\n') || (ch == '\r')) {
            break;
        }
    }

    return len;
}



int tty_control (device_t * dev, int cmd, int arg0, int arg1) {
    tty_t * tty = get_tty(dev);
    switch (cmd) 
    {
    case TTY_CMD_ECHO:
        if (arg0) {
            tty->iflags |= TTY_IECHO;
        } else {
            tty->iflags &= ~TTY_IECHO;
        }
        break;

    default:
        break;
    }
    return 0;
}

void tty_close (device_t * dev) {

}

void tty_in(char ch) {
    tty_t * tty = tty_devs + curr_tty;

    if (sem_count(&tty->isem) >= TTY_IBUF_SIZE) {
        // 说明缓存数据已满
        return;
    }

    tty_fifo_put(&tty->ififo, ch);
    sem_notify(&tty->isem);

}

// 选择tty
void tty_select (int tty) {
    if (tty != curr_tty) {
        console_select(tty);
        curr_tty = tty;
    }
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