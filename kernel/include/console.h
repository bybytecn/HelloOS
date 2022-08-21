#ifndef CONSOLE_H
#define CONSOLE_H

#include "../../common/include/types.h"
#define do_div(n, base)                                      \
    ({                                                       \
        unsigned long __upper, __low, __high, __mod, __base; \
        __base = (base);                                     \
        asm(""                                               \
            : "=a"(__low), "=d"(__high)                      \
            : "A"(n));                                       \
        __upper = __high;                                    \
        if (__high)                                          \
        {                                                    \
            __upper = __high % (__base);                     \
            __high = __high / (__base);                      \
        }                                                    \
        asm("divl %2"                                        \
            : "=a"(__low), "=d"(__mod)                       \
            : "rm"(__base), "0"(__low), "1"(__upper));       \
        asm(""                                               \
            : "=A"(n)                                        \
            : "a"(__low), "d"(__high));                      \
        __mod;                                               \
    })
typedef enum real_color
{
    rc_black = 0,
    rc_blue = 1,
    rc_green = 2,
    rc_cyan = 3,
    rc_red = 4,
    rc_magenta = 5,
    rc_brown = 6,
    rc_light_grey = 7,
    rc_dark_grey = 8,
    rc_light_blue = 9,
    rc_light_green = 10,
    rc_light_cyan = 11,
    rc_light_red = 12,
    rc_light_magenta = 13,
    rc_light_brown = 14, // yellow
    rc_white = 15
} real_color_t;

// 清屏操作
void console_clear();

// 屏幕输出一个字符(带颜色)
void console_putc_color(char c, real_color_t back, real_color_t fore);

// 屏幕打印一个以 \0 结尾的字符串(默认黑底白字)
void console_write(char *cstr);

// 屏幕打印一个以 \0 结尾的字符串(带颜色)
void console_write_color(char *cstr, real_color_t back, real_color_t fore);

// 屏幕输出一个十六进制的整型数
void console_write_hex(uint32_t n, real_color_t back, real_color_t fore);

// 屏幕输出一个十进制的整型数
void console_write_dec(uint32_t n, real_color_t back, real_color_t fore);

// 屏幕输出一个十六进制的整型数 64位
void console_write64_hex(uint64_t n, real_color_t back, real_color_t fore);

// 屏幕输出一个十进制的整型数 64位
void console_write64_dec(uint64_t n, real_color_t back, real_color_t fore);

//结合上面函数实现的多参数
void kprintf(char *format, ...);
#endif