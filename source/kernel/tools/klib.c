#include "comm/types.h"
#include "tools/klib.h"

// 字符串复制
void kernel_strcpy(char * dest, const char * src) {
    if (!dest || !src) {
        return;
    }

    while (*dest && *src) {
        *dest++ = *src++;
    }

    *dest = '\0';  // 结束字符
}

// 字符串复制
void kernel_strncpy(char * dest, const char * src, int size) {
    if (!dest || !src || !size) {
        return;
    }

    char * d = dest;
    const char * s = src;
    while((size-- > 0) && (*s)) {
        *d++ = *s++;
    }

    if (size == 0) {
        *(d - 1) = '\0';
    } else {
        *d = '\0';
    }

}

// 字符串比较
int kernel_strncmp(const char * s1, const char * s2, int size) {
    if (!s1 || !s2) {
        return -1;
    }

    while (*s1 && *s2 && (*s1 == *s2) && size) {
        s1++;
        s2++;
    }

    // s1 = "abc"
    // s2 = "abcdef"  这样认为相同，比到长度最小的看想不相同，这里是这样实现的。

    return !((*s1 == '\0') || (*s2 == '\0') || (*s1 == *s2));

}

// 求字符串长度
int kernel_strlen(const char * str) {
    if (!str) {
        return 0;
    }

    const char * c = str;
    int len = 0;
    while(*c++) {
        len++;
    }

    return len;
}


void kernel_memcpy(void * dest, void * src, int size) {
    if (!dest || !src || !size) {
        return;
    }

    uint8_t * s = (uint8_t *)src;
    uint8_t * d = (uint8_t *)dest;
    while(size--) {
        *d++ = *s++;
    }
}

void kernel_memset(void * dest, uint8_t v, int size) {
    if (!dest || !size) {
        return;
    }

    uint8_t * d = (uint8_t *)dest;
    while(size --) {
        *d++ = v;
    }

}
int kernel_memcmp(void * d1, void * d2, int size) {
    if (!d1 || !d2 || !size) {
        return 1;
    }

    uint8_t * p_d1 = (uint8_t *)d1;
    uint8_t * p_d2 = (uint8_t *)d2;
    while(size--) {
        if (*p_d1++ != *p_d2++) {
            return 1;
        }
    }

    return 0;
}

void kernel_itoa(char * buf, int num, int base) {
    // 转换字符索引[-15, -14, ...-1, 0, 1, ...., 14, 15]
    static const char * num2ch = {"FEDCBA9876543210123456789ABCDEF"};
    char * p = buf;
    int old_num = num;

    // 仅支持部分进制
    if ((base != 2) && (base != 8) && (base != 10) && (base != 16)) {
        *p = '\0';
        return;
    }

    if ((num < 0) && (base == 10)) {
        *p++ = '-';
    }

    // 一位一位的转换
    do {
        char ch = num2ch[num % base + 15];
        *p++ = ch;
        num /= base;
    }while(num);

    *p-- = '\0';

    // 将转换结果逆序，生成最终的结果
    char * start = (old_num > 0) ? buf : buf + 1;
    while (start < p) {
        char ch = *start;
        *start = *p;
        *p-- = ch;
        start++;
    }
}

/**
 *  格式化字符串到缓存中
 */
void kernel_sprintf(char * buffer, const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    kernel_vsprintf(buffer, fmt, args);
    va_end(args);
}


/**
 * 格式化字符串
 */
void kernel_vsprintf(char * buffer, const char * fmt, va_list args) {
    enum {NORMAL, READ_FMT} state = NORMAL;
    char ch;
    char * curr = buffer;
    while ((ch = *fmt++)) {
        switch (state) {
            // 普通字符
            case NORMAL:
                if (ch == '%') {
                    state = READ_FMT;
                } else {
                    *curr++ = ch;
                }
                break;
            // 格式化控制字符，只支持部分
            case READ_FMT:
                if (ch == 'd') {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 10);
                    curr += kernel_strlen(curr);

                } else if (ch == 'x') {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 16);
                    curr += kernel_strlen(curr);

                } else if (ch == 'c') {
                    char c = va_arg(args, int);
                    *curr++ = c;
                    
                } else if (ch == 's') {
                    const char * str = va_arg(args, char *);
                    int len = kernel_strlen(str);
                    while (len--) {
                        *curr++ = *str++;
                    }
                }
                state = NORMAL;
                break;
        }
    }
}









