/**
 * 键盘设备处理
 */
#ifndef KBD_H
#define KBD_H

#include "comm/types.h"

#define KEY_RSHIFT      0x36
#define KEY_LSHIFT      0x2A
#define KEY_CAPS        0x3A

#define KEY_E0          0xE0        // E0编码
#define KEY_E1          0xE1        // E1编码KEY_CTRL

/**
 * 特殊功能键
 */
#define KEY_CTRL 		0x1D		// E0, 1D或1d
#define KEY_RSHIFT		0x36
#define KEY_LSHIFT 		0x2A
#define KEY_ALT 		0x38		// E0, 38或38

#define KEY_F1			(0x3B)
#define KEY_F2			(0x3C)
#define KEY_F3			(0x3D)
#define KEY_F4			(0x3E)
#define KEY_F5			(0x3F)
#define KEY_F6			(0x40)
#define KEY_F7			(0x41)
#define KEY_F8			(0x42)
#define KEY_F9			(0x43)
#define KEY_F10			(0x44)
#define KEY_F11			(0x57)
#define KEY_F12			(0x58)


/**
 * 键盘扫描码表单元类型
 * 每个按键至多有两个功能键值
 * code1：无shift按下或numlock灯亮的值，即缺省的值
 * code2：shift按下或者number灯灭的值，即附加功能值
 */
typedef struct _key_map_t {
    uint8_t normal;     // 普通功能 normal是没有shift键按下，或者没有numlock按下时默认的键值
    uint8_t func;       // 第二功能 func是按下shift或者numlock按下时的键值
}key_map_t;

/**
 * 状态指示灯
 */
typedef struct _kbd_state_t {

    int caps_lock : 1;          // 大写状态
    int lshift_press : 1;       // 左shift按下
    int rshift_press : 1;       // 右shift按下
    int lalt_press : 1;         // alt按下
    int ralt_press : 1;         // alt按下
    int lctrl_press : 1;        // ctrl键按下
    int rctrl_press : 1;        // ctrl键按下
}kbd_state_t;

// https://wiki.osdev.org/%228042%22_PS/2_Controller
#define KBD_PORT_DATA       0x60
#define KBD_PORT_STAT       0x64
#define KBD_PORT_CMD        0x64

#define KBD_STAT_RECV_READY  (1 << 0)


void kbd_init (void);   // 键盘初始化
void exception_handler_kbd (void);


#endif