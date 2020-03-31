/*  IO.H -  Provides interface for the kernel to change memory, interface with ports.
 *
 *  Author: Crupette
 * */

#ifndef IO_H
#define IO_H 1

#include <stddef.h>
#include <stdint.h>

/*  Writes / reads a byte to/from an I/O port
 *  port: Port to write to
 *  data: Data to pass
 * */
extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);

/*  Copies n bytes from b to a using SIMD
 * */
void memcpy(void *a, void *b, size_t n);

/*  Sets n bytes starting at a to b using SIMD
 * */
void memset(void *a, uint8_t b, size_t n);

#endif
