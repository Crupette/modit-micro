#include "string.h"
#include <stdbool.h>

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

char *strstr(const char *hs, const char *n){
	const char *beg = hs;
	const char *nind = n;
	const char *hsind = hs;
	bool found = true;
	while(*hsind != NULL){
		beg = hsind;
		while(*beg == *n && *nind == *hsind){
			hsind++;
			nind++;
		}
		if(*nind == NULL){
			return beg;
		}

		hsind++;
		nind++;
	}
	return NULL;
}

char *strcpy(char *dest, const char *src){
	memcpy(dest, src, strlen(src));
	return dest;
}
