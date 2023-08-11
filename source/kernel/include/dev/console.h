#ifndef CONSOLE_H
#define CONSOLE_H

#include "comm/types.h"

#define CONSOLE_DISP_ADDR   0xb8000
#define CONSOLE_DISP_END    (0xb8000 + 32*1024)
#define CONSOLE_ROW_MAX     25
#define CONSOLE_COL_MAX     80   

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
typedef struct _console_t {
    disp_char_t * disp_base;
    int cursor_row, cursor_col; // 光标所在的行和列
    int display_rows, display_cols;
    color_t foreground, background; // 颜色
}console_t;


int console_init (void);
int console_write (int console, char * data, int size);
void console_close (int console);


#endif