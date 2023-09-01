/**
 * 终端显示部件
 * 只支持VGA模式
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include "comm/types.h"
#include "dev/tty.h"

// https://wiki.osdev.org/Printing_To_Screen
#define CONSOLE_DISP_ADDR   0xb8000
#define CONSOLE_DISP_END    (0xb8000 + 32*1024)     // 显存的结束地址
#define CONSOLE_ROW_MAX     25                      // 行数
#define CONSOLE_COL_MAX     80                      // 最大列数    

#define ASCII_ESC       0x1b            // \033   ESC ascii码       
#define ESC_PARAM_MAX   10              // 最多支持的ESC [ 参数数量

// 各种颜色
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


/**
 * @brief 显示字符
 */
typedef union  _disp_char_t {
    struct {
        char c;                     // 显示的字符
        char foreground : 4;        // 前景色            
        char background : 3;        // 背景色
    };
    uint16_t v;
}disp_char_t;

// 描述控制台
// ESC 7, 8
// ESC(\033) [p0:p1 m
/**
 * 终端显示部件
 */
typedef struct _console_t {
    enum {
        CONSOLE_WRITE_NORMAL, // 正在写普通字符 普通模式
        CONSOLE_WRITE_ESC,      // 正在写 ESC 开头序列      ESC转义序列
        CONSOLE_WRITE_SQUARE,   // 对方括号进行处理         ESC [接收状态
    }write_state;

    disp_char_t * disp_base;
    int cursor_row, cursor_col; // 光标所在的行和列     当前编辑的行和列
    int display_rows, display_cols;         // 显示界面的行数和列数
    color_t foreground, background; // 颜色     前后景色

    int old_cursor_col, old_cursor_row;     // 保存光标位置
    int esc_param[ESC_PARAM_MAX];           // ESC [ ;;参数数量
    int curr_param_index;   // 当前保存参数的位置，用于esc_param[ESC_PARAM_MAX]这个数组里面的索引

}console_t;


int console_init (int idx);
int console_write (tty_t * tty);
void console_close (int console);
void console_select (int idx);      // 切换屏幕

#endif