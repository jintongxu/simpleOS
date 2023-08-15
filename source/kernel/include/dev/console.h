#ifndef CONSOLE_H
#define CONSOLE_H

#include "comm/types.h"
#include "dev/tty.h"

#define CONSOLE_DISP_ADDR   0xb8000
#define CONSOLE_DISP_END    (0xb8000 + 32*1024)
#define CONSOLE_ROW_MAX     25
#define CONSOLE_COL_MAX     80   

#define ASCII_ESC       0x1b            // \033
#define ESC_PARAM_MAX   10

typedef enum _color_t {
    COLOR_Black = 0,
    COLOR_Blue,
    COLOR_Green,
    COLOR_Cyan,
    COLOR_Red,
    COLOR_Magenta,
    COLOR_Brown,
    COLOR_Gray,
    COLOR_DarkGray,
    COLOR_Light_Blue,
    COLOR_Light_Green,
    COLOR_Light_Cyan,
    COLOR_Light_Red,
    COLOR_Light_Magenta,
    COLOR_Yellow,
    COLOR_White,
}color_t;

// 显示结构
typedef union  _disp_char_t {
    struct {
        char c;
        char foreground : 4;
        char background : 3;
    };
    uint16_t v;
}disp_char_t;

// 描述控制台
// ESC 7, 8
// ESC(\033) [p0:p1 m
typedef struct _console_t {
    enum {
        CONSOLE_WRITE_NORMAL, // 正在写普通字符
        CONSOLE_WRITE_ESC,      // 正在写 ESC 开头序列
        CONSOLE_WRITE_SQUARE,   // 对方括号进行处理
    }write_state;

    disp_char_t * disp_base;
    int cursor_row, cursor_col; // 光标所在的行和列
    int display_rows, display_cols;
    color_t foreground, background; // 颜色

    int old_cursor_col, old_cursor_row;     // 保存光标位置
    int esc_param[ESC_PARAM_MAX];
    int curr_param_index;   // 当前保存参数的位置，用于esc_param[ESC_PARAM_MAX]这个数组里面的索引

}console_t;


int console_init (int idx);
int console_write (tty_t * tty);
void console_close (int console);
void console_select (int idx);      // 切换屏幕

#endif