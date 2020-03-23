#ifndef PRINT_H
#define PRINT_H 1

#include <stdarg.h>

/*	Prints to the standard VGA buffer using printf syntax
 * */
void vga_printf(const char *fmt, ...);

#endif
