#include "kernel/io.h"

void memcpy(void *a, void *b, size_t n){
	//vga_printf("Copying %x bytes from %p to %p\n", n, b, a);
	asm volatile("cld;rep movsb": "=c"((int){0}): "D"(a), "S"(b), "c"(n): "flags", "memory");
}

void memset(void *a, uint8_t b, size_t n){
	asm volatile("cld;rep stosb": "=c"((int){0}): "D"(a), "a"(b), "c"(n): "flags", "memory");
}
