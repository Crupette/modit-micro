#ifndef STD_LIB_H
#define STD_LIB_H 1

#include <stdint.h>
#include <stddef.h>

/*  Allocates bytes and returns pointer to memory
 *  DOES NOT INITIALIZE MEMORY
 *  size:   Size of memory to allocate in bytes
 *  r:      Pointer to new memory. 0 if failed or size passed is 0
 * */
void *malloc(size_t size);

/*  Reallocates a buffer to grow it's size
 *  if size is 0, frees the memory at pre
 *  ptr:    Memory to reallocate
 *  size:   Size to reallocate to
 *  r:      Pointer to beginning of new memory block
 * */
void *realloc(void *ptr, size_t size);

/*  Frees the memory space allocated by malloc / other functions
 *  ptr:    Pointer to memory to free
 * */
void free(void *ptr);

#endif
