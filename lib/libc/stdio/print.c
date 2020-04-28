#include <stdio.h>
#include <string.h>

int putchar(const char c){
    return fputc(stdout, c);
}

int fputc(FILE *restrict stream, const char c){
    fwrite(&c, 1, 1, stream);
    fflush(stream);
    return (int)c;
}

int puts(const char *s){
    return fputs(stdout, s);
}

int fputs(FILE *restrict stream, const char *s){
    int i = fwrite(s, strlen(s), 1, stream);
    i |= fwrite("\n", 1, 1, stream);
    fflush(stream);
    return i;
}

int printf(const char *restrict format, ...){
    va_list ap;
    va_start(ap, format);

    int r = vfprintf(stdout, format, ap);

    va_end(ap);
    return r;  
}

int fprintf(FILE *restrict stream, const char *restrict format, ...){
    va_list ap;
    va_start(ap, format);

    int r = vfprintf(stream, format, ap);

    va_end(ap);
    return r;   
}

/*
int sprintf(char *restrict str, const char *restrict format, ...){
    
}

int snprintf(char *restrict str, size_t size, const char *restrict format, ...){

}
*/

int vprintf(const char *restrict format, va_list ap){
    return vfprintf(stdout, format, ap);
}

#define PRINTF_FLAG_SIGNED 1

static int __printi_loop(FILE *restrict stream, int32_t n, uint8_t base){
    int c = 0;
    if(n >= base){
        c += __printi_loop(stream, n / base, base);
    }
    fputc(stream, (n % base) + ((n % base) >= 10 ? ('A'-10) : '0'));
    return c;
}

static int __printi(FILE *restrict stream, int32_t n, uint8_t base, uint32_t flags){
    int c = 0;
    if(n < 0 && (flags & PRINTF_FLAG_SIGNED)){
        fputc(stream, '-');
        n = 0 - n;
        c++;
    }
    c += __printi_loop(stream, n, base);
    return c;
}

int vfprintf(FILE *restrict stream, const char *restrict format, va_list ap){
    char c;
    int count;
    int base;
   
    while(1){
        while((c = *format++) != '%'){
            if(c == 0){
                fflush(stream);
                return count;
            }
            fputc(stream, c);
            count++;
        }
        //TODO: Flags
        switch(c = *format++){
            case 'd':
            case 'i':
                count += __printi(stream, va_arg(ap, int32_t), 10, 1);
            break;
            case 'o':
                count += __printi(stream, va_arg(ap, uint32_t), 8, 0);
            break;
            case 'u':
                count += __printi(stream, va_arg(ap, uint32_t), 10, 0);
            break;
            case 'x':
            case 'X':
            case 'p':
                count += __printi(stream, va_arg(ap, uint32_t), 16, 0);
            break;
            case 's':
            {
                const char *sptr = va_arg(ap, const char*);
                count += fwrite(sptr, strlen(sptr), 1, stream);
            }
            break;
        }
    }
}

/*
int vsprintf(char *restrict str, const char *restrict format, va_list ap){
    
}

*/
