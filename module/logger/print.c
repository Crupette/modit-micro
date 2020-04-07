#include "module/heap.h"

#include "kernel/logging.h"
#include "kernel/io.h"

#include <stdarg.h>

char *l_itoa(char *buf, uint32_t n, uint32_t base, bool s, bool caps){
    size_t i = 0, dec = 1;
    if(n < 0 && s){
        n = 0 - n;
        *buf++ = '-';
        i++;
    }
    while(n / dec >= base){
        dec *= base;
    }
    while(dec != 0){
        int d = n / dec;
        n %= dec;
        dec /= base;
        if(i > 0 || d > 0 || dec == 0){
            *buf++ = d + (d < 10 ? '0' : (caps ? 'A' - 10 : 'a' - 10));
            i++;
        }
    }
    *buf = 0;
    return buf;
}

int l_vsprintf(char *buf, const char *fmt, va_list ap){
    char *obuf = buf;
    for(const char *fi = fmt; *fi; fi++){
        if(*fi != '%'){
            *buf = *fi;
            buf++;
            continue;
        }
        ++fi;

        switch(*fi){
            case 'x':
                *buf++ = '0';
                *buf++ = 'x';
                buf = l_itoa(buf, va_arg(ap, uint32_t), 16, false, false);
                break;
            case 'p':
                __attribute__((fallthrough));
            case 'X':
                *buf++ = '0';
                *buf++ = 'x';
                buf = l_itoa(buf, va_arg(ap, uint32_t), 16, false, false);
                break;
            case 'o':
                *buf++ = '0';
                buf = l_itoa(buf, va_arg(ap, uint32_t), 8, false, false);
                break;
            case 'u':
                buf = l_itoa(buf, va_arg(ap, uint32_t), 10, false, false);
                break;               
            case 'i':
                buf = l_itoa(buf, va_arg(ap, uint32_t), 10, true, false);
                break;               
            case 's':
                {
                    char *n = va_arg(ap, char*);
                    for(; *n; n++){
                        *buf = *n;
                        buf++;
                    }
                }
        }
    }
    *buf = 0;
    //vga_printf("Got buffer %s\n", obuf);
    return (uintptr_t)buf - (uintptr_t)obuf;
}

int l_sprintf(char *buf, const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int c = l_vsprintf(buf, fmt, ap);

    va_end(ap);
    return c;
}
