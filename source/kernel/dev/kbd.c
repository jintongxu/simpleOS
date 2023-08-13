#include "dev/kbd.h"
#include "cpu/irq.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"
#include "tools/klib.h"


static kbd_state_t kbd_state;

/**
 * 键盘映射表，分3类
 * normal是没有shift键按下，或者没有numlock按下时默认的键值
 * func是按下shift或者numlock按下时的键值
 * esc是以esc开头的的键值
 */
static const key_map_t map_table[256] = {
        [0x2] = {'1', '!'},
        [0x3] = {'2', '@'},
        [0x4] = {'3', '#'},
        [0x5] = {'4', '$'},
        [0x6] = {'5', '%'},
        [0x7] = {'6', '^'},
        [0x08] = {'7', '&'},
        [0x09] = {'8', '*' },
        [0x0A] = {'9', '('},
        [0x0B] = {'0', ')'},
        [0x0C] = {'-', '_'},
        [0x0D] = {'=', '+'},
        [0x0E] = {'\b', '\b'},
        [0x0F] = {'\t', '\t'},
        [0x10] = {'q', 'Q'},
        [0x11] = {'w', 'W'},
        [0x12] = {'e', 'E'},
        [0x13] = {'r', 'R'},
        [0x14] = {'t', 'T'},
        [0x15] = {'y', 'Y'},
        [0x16] = {'u', 'U'},
        [0x17] = {'i', 'I'},
        [0x18] = {'o', 'O'},
        [0x19] = {'p', 'P'},
        [0x1A] = {'[', '{'},
        [0x1B] = {']', '}'},
        [0x1C] = {'\n', '\n'},
        [0x1E] = {'a', 'A'},
        [0x1F] = {'s', 'B'},
        [0x20] = {'d',  'D'},
        [0x21] = {'f', 'F'},
        [0x22] = {'g', 'G'},
        [0x23] = {'h', 'H'},
        [0x24] = {'j', 'J'},
        [0x25] = {'k', 'K'},
        [0x26] = {'l', 'L'},
        [0x27] = {';', ':'},
        [0x28] = {'\'', '"'},
        [0x29] = {'`', '~'},
        [0x2B] = {'\\', '|'},
        [0x2C] = {'z', 'Z'},
        [0x2D] = {'x', 'X'},
        [0x2E] = {'c', 'C'},
        [0x2F] = {'v', 'V'},
        [0x30] = {'b', 'B'},
        [0x31] = {'n', 'N'},
        [0x32] = {'m', 'M'},
        [0x33] = {',', '<'},
        [0x34] = {'.', '>'},
        [0x35] = {'/', '?'},
        [0x39] = {' ', ' '},
};

// 键盘初始化
void kbd_init (void) {
    kernel_memset(&kbd_state, 0, sizeof(kbd_state));
    irq_install(IRQ1_KEYBOARD, (irq_handler_t)exception_handler_kbd);
    irq_enable(IRQ1_KEYBOARD);
}

// 判断按下的
static inline int is_make_code(uint8_t key_code) {
    return !(key_code & 0x80);
}

static inline int get_key(uint8_t key_code) {
    return key_code & 0x7f;
}


// 处理单字符的标准键
static void do_normal_key (uint8_t raw_code) {
    char key = get_key(raw_code);
    int is_make = is_make_code(raw_code);

    switch (key) {
        case KEY_RSHIFT:
            kbd_state.rshift_press = is_make ? 1 : 0;
            break;
        case KEY_LSHIFT:
            kbd_state.lshift_press = is_make ? 1 : 0;
            break;
        case KEY_CAPS:      // // 大小写键，设置大小写状态
            if (is_make) {
                kbd_state.caps_lock = ~kbd_state.caps_lock;
            }
            break;
        default:
            if (is_make) {
                if (kbd_state.rshift_press || kbd_state.lshift_press) {
                    key = map_table[key].func;
                } else {
                    key = map_table[key].normal;
                }

                // 根据caps再进行一次字母的大小写转换
                if (kbd_state.caps_lock) {
                    if ((key >= 'A') && (key <= 'Z')) {
                        // 大写转小写
                        key = key - 'A' + 'a';
                    } else if ((key >= 'a') && (key <= 'z')) {
                        // 小写转大小
                        key = key - 'a' + 'A';
                }
            }


                log_printf("key: %c", key);
            }
            break;
    }
}


// 按键中断处理程序
void do_handler_kbd (exception_frame_t * frame) {
    uint32_t status = inb(KBD_PORT_STAT);
    if (!(status & KBD_STAT_RECV_READY)) {
        pic_send_eoi(IRQ1_KEYBOARD);
        return;
    }

    uint8_t raw_code = inb(KBD_PORT_DATA);
    do_normal_key(raw_code);

    

    pic_send_eoi(IRQ0_TIMER);


}
