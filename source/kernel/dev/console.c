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
static void erase_rows (console_t * console, int start, int end) {
    volatile disp_char_t * disp_start = console->disp_base + console->display_cols * start;
    volatile disp_char_t * disp_end = console->disp_base + console->display_cols * (end + 1);

    while (disp_start < disp_end) {
        disp_start->c = ' ';
        disp_start->foreground = console->foreground;
        disp_start->background = console->background;

        disp_start++;
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


static void clear_esc_param (console_t  * console) {
    kernel_memset(console->esc_param, 0, sizeof(console->esc_param));
    console->curr_param_index = 0;
}


//  写入以ESC开头的序列
//  ESC(\033) 7 8
//  ESC [pn:pn1
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
    case '[':
        clear_esc_param(console);       // 对参数列表进行清空
        console->write_state = CONSOLE_WRITE_SQUARE;
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

// 设置字体颜色
static void set_font_style (console_t * console) {
    static const color_t color_table[] = {
        COLOR_Black, COLOR_Red, COLOR_Green, COLOR_Yellow,
        COLOR_Blue, COLOR_Magenta, COLOR_Cyan, COLOR_White,
    };

    for (int i = 0; i <= console->curr_param_index; i++) {
        int param = console->esc_param[i];
        if ((param >= 30) && (param <= 37)) {
            // 设置前景色
            console->foreground = color_table[param - 30];
        } else if ((param >= 40) && (param <= 47)) {
            // 设置背景色
            console->background = color_table[param - 40];
        } else if (param == 39) {
            // 默认的
            console->foreground = COLOR_White;
        } else if (param == 49) {
            console->background = COLOR_Black;
        }
    }
}


// 光标向 左 移动
static void move_left (console_t * console, int n) {
    if (n == 0) {
        n = 1;
    }

    int col = console->cursor_col - n;
    console->cursor_col = (col >= 0) ? col : 0;   // 判断是否超过左边边界
}


// 光标向 右 移动
static void move_right (console_t * console, int n) {
    if (n == 0) {
        n = 1;
    }

    int col = console->cursor_col + n;
    // 如果列超出右边界了
    if (col >= console->display_cols) {
        console->cursor_col = console->display_cols - 1;
    } else {
        console->cursor_col = col;
    }
}


// 移动光标
static void move_cursor (console_t * console) {
    console->cursor_row = console->esc_param[0];
    console->cursor_col = console->esc_param[1];
}


// 擦除字符操作
static void erase_in_display(console_t * console) {
	if (console->curr_param_index <= 0) {
		return;
	}

	int param = console->esc_param[0];
	if (param == 2) {
		// 擦除整个屏幕
		erase_rows(console, 0, console->display_rows - 1);
        console->cursor_col = console->cursor_row = 0;
	}
}


// 处理ESC [Pn;Pn 开头的字符串
static void write_esc_square (console_t * console, char c) {
     // 接收参数
    if ((c >= '0') && (c <= '9'))
    {
        // 解析当前参数
        int *param = &console->esc_param[console->curr_param_index];
        *param = *param * 10 + c - '0';
    }
    else if ((c == ';') && console->curr_param_index < ESC_PARAM_MAX)
    {
        // 参数结束，继续处理下一个参数
        console->curr_param_index++;
    }
    else
    {
        // 结束上一字符的处理
        console->curr_param_index++;

        // 已经接收到所有的字符，继续处理
        switch (c)
        {
        case 'm': // 设置字符属性
            set_font_style(console);
            break;
        case 'D': // 光标左移n个位置 ESC [Pn D
            move_left(console, console->esc_param[0]);
            break;
        case 'C':
            move_right(console, console->esc_param[0]);
            break;
        case 'H':
        case 'f':
            move_cursor(console);
            break;
        case 'J':
            erase_in_display(console);
            break;
        }
        console->write_state = CONSOLE_WRITE_NORMAL;
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
            case CONSOLE_WRITE_SQUARE:
                write_esc_square(c, ch);
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