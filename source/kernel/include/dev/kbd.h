#ifndef KBD_H
#define KBD_H

#include "comm/types.h"

#define KEY_RSHIFT      0x36
#define KEY_LSHIFT      0x2A

typedef struct _key_map_t {
    uint8_t normal;     // 普通功能 normal是没有shift键按下，或者没有numlock按下时默认的键值
    uint8_t func;       // 第二功能 func是按下shift或者numlock按下时的键值
}key_map_t;


typedef struct _kbd_state_t {
    int lshift_press : 1;
    int rshift_press : 1;
}kbd_state_t;

#define KBD_PORT_DATA       0x60
#define KBD_PORT_STAT       0x64
#define KBD_PORT_CMD        0x64

#define KBD_STAT_RECV_READY  (1 << 0)


void kbd_init (void);   // 键盘初始化
void exception_handler_kbd (void);


#endif