#include <string.h>

void *memset(void *s, int c, size_t n){
     asm volatile("cld;rep stosb": "=c"((int){0}): "D"(s), "a"(c), "c"(n): "flags", "memory");   
    return s;
}

void *memcpy(void *dest, const void *src, size_t n){
    asm volatile("cld;rep movsb": "=c"((int){0}): "D"(dest), "S"(src), "c"(n): "flags", "memory");
     return dest;
}
