#include "kernel/print.h"
#include "kernel/io.h"
#include "kernel/vgaterm.h"
#include <stdint.h>

char *itoa(char *buffer, int num, uint8_t base, bool negative){
    size_t i = 0, decimal = 1;
    if(num < 0 && negative){
        num = 0 - num;
        *buffer++ = '-';
        i++;
    }
    while(num / decimal >= base){
        decimal *= base;
    }
    while(decimal != 0){
        int digit = num / decimal;
        num %= decimal;
        decimal /= base;
        if(i > 0 || digit > 0 || decimal == 0){
            *buffer++ = digit + (digit < 10 ? '0' : 'A' - 10);
            i++;
        }
    }
    *buffer = 0;
    return buffer;
}

void vga_printf(const char *fmt, ...){
    char buf[256];
    char *buffer = buf;

    memset(buf, 0, 256);

    va_list ap;
    va_start(ap, fmt);

    for(const char *fmti = fmt; *fmti; fmti++){
        if(*fmti != '%'){
            *buffer = *fmti;
            buffer++;
            continue;
        }
        ++fmti;
        switch(*fmti){
            case 'p':
            __attribute__((fallthrough));
            case 'x':
            __attribute__((fallthrough));
            case 'X':
                buffer = itoa(buffer, va_arg(ap, uint32_t), 16, false);
            break;
            case 'o':
                buffer = itoa(buffer, va_arg(ap, uint32_t), 8, false);
            break;
            case 'u':
            __attribute__((fallthrough));
            case 'd':
            __attribute__((fallthrough));
            case 'i':
                buffer = itoa(buffer, va_arg(ap, uint32_t), 10, true);
            break;
            case 's':
            {
                char *new = va_arg(ap, char*);
                for(; *new; new++){
                    *buffer = *new;
                    buffer++;
                }
            }
        }
    }
    *buffer = 0;

    vgaterm_putstr(buf);

    va_end(ap);
}
