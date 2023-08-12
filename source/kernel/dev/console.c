#include "dev/console.h"
#include "tools/klib.h"

#define CONSOLE_NR      1

static console_t console_buf[CONSOLE_NR];

// 擦除操作
static void erase_rows  (console_t * console, int start, int end) {
    volatile disp_char_t * disp_start = console->disp_base + console->display_cols * start;
    volatile disp_char_t * disp_end = console->disp_base + console->display_cols * (end + 1);
    while(disp_start < disp_end) {
        disp_start->c = ' ';
        disp_start->foreground = console->foreground;
        disp_start->background = console->background;

        disp_start ++;
    }
}

// 滚屏操作 lines: 滚动几行
static void scroll_up (console_t * console, int lines) {
    disp_char_t * dest = console->disp_base;
    disp_char_t * src = console->disp_base + console->display_cols * lines;
    uint32_t size = (console->display_rows - lines) * console->display_cols * sizeof(disp_char_t);
    kernel_memcpy(dest, src, size);

    erase_rows(console, console->display_rows - lines, console->display_rows - 1);  // 清空行
    console->cursor_row -= lines;

}


// 将光标移动到0列（即移动到该光标所在行的开头）
static void move_to_col0(console_t * console) {
    console->cursor_col = 0;
}

// 将光标移动到下一行
static void move_next_line (console_t * console) {
    console->cursor_row ++;
    if (console->cursor_row >= console->display_rows) {
        // 如果行超了就要滚动屏幕
        scroll_up(console, 1);
    }
}

static void move_forward (console_t * console, int n) {
    for (int i = 0; i < n; i++) {
        // 向右移动就是列数+1，然后判断超没超
        if (++console->cursor_col >= console->display_cols) {
            console->cursor_row++;
            console->cursor_col = 0;

            // 如果行数超过了最大值，上滚1行
            if (console->cursor_row >= console->display_rows) {
                scroll_up(console, 1);
            }
        }
    }
}

static void show_char(console_t * console, char c) {
    int offset = console->cursor_col + console->cursor_row * console->display_cols;
    disp_char_t * p = console->disp_base + offset;
    p->c = c;
    p->foreground = console->foreground;
    p->background = console->background;
    move_forward(console, 1);   // 前移光标
}

static void clear_display (console_t * console) {
    int size = console->display_cols * console->display_rows;

    disp_char_t * start = console->disp_base;
    for (int i = 0; i < size; i++, start++) {
        start->c = ' ';
        start->foreground = console->foreground;
        start->background = console->background;
    }
}

int console_init (void) {
    for (int i = 0; i < CONSOLE_NR; i++) {
        console_t * console = console_buf + i;

        console->cursor_row = 0;
        console->cursor_col = 0;
        console->display_cols = CONSOLE_COL_MAX;
        console->display_rows = CONSOLE_ROW_MAX;
        console->foreground = COLOR_White;
        console->background = COLOR_Black;

        console->disp_base = (disp_char_t *)CONSOLE_DISP_ADDR + i * (CONSOLE_COL_MAX * CONSOLE_ROW_MAX);

        clear_display(console);
    }
}

int console_write (int console, char * data, int size) {
    console_t * c = console_buf + console;
    int len;

    for (len = 0; len < size; len++) {
        char ch = *data++;
        switch (ch) {
            case '\n':
                move_to_col0(c);      // 将光标移动到第零列
                move_next_line(c);    // 将光标移动到下一行
                break;
            default:
                show_char(c, ch);
                break;
        }
       
    }
    return len;
}

void console_close (int console) {
    
}