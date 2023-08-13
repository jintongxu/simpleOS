#include "dev/console.h"
#include "tools/klib.h"
#include "comm/cpu_instr.h"

#define CONSOLE_NR      1

static console_t console_buf[CONSOLE_NR];

// 获取当前光标位置
static int read_cursor_pos (void) {
    int pos;

    outb(0x3D4, 0xF);
    pos = inb(0x3d5);
    outb(0x3D4, 0xE);  
    pos |= inb(0x3d5) << 8;

    return pos;
}

// 更新鼠标的位置
static int update_cursor_pos (console_t * console) {
    uint16_t pos = console->cursor_row * console->display_cols + console->cursor_col;


    outb(0x3D4, 0xF);
    outb(0x3d5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0xE);  
    outb(0x3d5, (uint8_t)((pos >> 8) & 0xFF));

    return pos;
}

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

// 向左移动光标
static int move_backword (console_t * console, int n) {
    int status = -1;

    for (int i = 0; i < n; i++) {
        if (console->cursor_col > 0) {
            // 非列超始处,可回退
            console->cursor_col--;
            status = 0;
        } else if (console->cursor_row > 0) {
            // 列起始处，但非首行，可回腿
            console->cursor_row--;
            console->cursor_col = console->display_cols - 1;
            status = 0;
        }
    }

    return status;
}

// 向后擦除一个字符
static void erase_backword (console_t * console) {
    if (move_backword(console, 1) == 0) {
        show_char(console, ' ');
        move_backword(console, 1);
    }
}

int console_init (void) {
    for (int i = 0; i < CONSOLE_NR; i++) {
        console_t * console = console_buf + i;

        console->display_cols = CONSOLE_COL_MAX;
        console->display_rows = CONSOLE_ROW_MAX;
        console->foreground = COLOR_White;
        console->background = COLOR_Black;

        int cursor_pos = read_cursor_pos();
        console->cursor_row = cursor_pos / console->display_cols;
        console->cursor_col = cursor_pos / console->display_cols;
        console->old_cursor_col = console->cursor_col;
        console->old_cursor_row = console->cursor_row;
        console->write_state = CONSOLE_WRITE_NORMAL;
     
        console->disp_base = (disp_char_t *)CONSOLE_DISP_ADDR + i * (CONSOLE_COL_MAX * CONSOLE_ROW_MAX);

        // clear_display(console);
    }
}


// 保存光标
void save_cursor (console_t * console) {
    console->old_cursor_col = console->cursor_col;
    console->old_cursor_row = console->cursor_row;
}

// 恢复光标
void restore_cursor (console_t * console) {
    console->cursor_col = console->old_cursor_col;
    console->cursor_row = console->old_cursor_row;
}


//  写入以ESC开头的序列
static void write_esc (console_t * console, char c) {
    switch (c)
    {
    case '7':
        save_cursor(console);
        console->write_state = CONSOLE_WRITE_NORMAL;
        break;
    case '8':
        restore_cursor(console);
        console->write_state = CONSOLE_WRITE_NORMAL;
        break;
    default:
        console->write_state = CONSOLE_WRITE_NORMAL;
        break;
    }
}


// 普通状态下的字符的写入处理
static void write_normal (console_t * console, char c) {
    switch (c) {
        case ASCII_ESC:
            console->write_state = CONSOLE_WRITE_ESC;
            break;
        case 0x7F:
            erase_backword(console);    // 往回删除一个字符
            break;
        case '\b':
            move_backword(console, 1);        // 光标向左移动一位
            break;
        case '\r':
            move_to_col0(console);
            break;
        case '\n':
            move_to_col0(console);      // 将光标移动到第零列
            move_next_line(console);    // 将光标移动到下一行
            break;
        default:
            if ((c >= ' ') && (c <= '~')) {
                show_char(console, c);
            }
            break;
    }
}

int console_write (int console, char * data, int size) {
    console_t * c = console_buf + console;
    int len;

    for (len = 0; len < size; len++) {
        char ch = *data++;
        switch (c->write_state) {
            case CONSOLE_WRITE_NORMAL:
                write_normal(c, ch);
                break;
            case CONSOLE_WRITE_ESC:
                write_esc(c, ch);
                break;
            default:
                break;
        }
       
    }

    update_cursor_pos(c);
    return len;
}

void console_close (int console) {
    
}