/*  SYSTEM.H - Setup for usermode environment (heap, permissions, etc)
 *
 *  Author: Crupette
 * */

#ifndef MICRO_SYSTEM_H
#define MICRO_SYSTEM_H

#include <stdint.h>

/*  Gets the current tasks permissions from the kernel
 * */
uint32_t micro_getperms(void);

#endif
