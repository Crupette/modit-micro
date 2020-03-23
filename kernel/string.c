#include "string.h"

size_t strlen(const char *s){
	register const char *i;
	for(i = s; *i;++i);
	return i - s;
}

int strcmp(const char *s1, const char *s2){
	const char *a = s1;
	const char *b = s2;
	while(*a && *b){
		if(*a != *b) return *a - *b;
		a++;
		b++;
	}
	return *a - *b;
}
