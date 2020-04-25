/*  PAGING.H - Wrapper for paging syscalls
 *
 *  Author: Crupette
 * */

#ifndef MICRO_PAGING_H
#define MICRO_PAGING_H 1

#include <stdint.h>
#include <stddef.h>

int micro_freepgs(void *virt, size_t cnt);
int micro_allocpgs(void *virt, size_t cnt, uint32_t flgs);
int micro_mappg(void *phys, void *virt, uint32_t flgs);
int micro_resvpg(void *phys);

#endif
