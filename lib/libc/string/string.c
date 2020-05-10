#include <string.h>

size_t strlen(const char *s){
    size_t i;
    for(i = 0; s[i]; i++){ }
    return i;
}

int strcmp(const char *s1, const char *s2){
    while(*s1 && *s2){
        if(*s1 != *s2) return *s2 - *s1;
        s1++;
        s2++;
    }
    return *s2 - *s1;
}
