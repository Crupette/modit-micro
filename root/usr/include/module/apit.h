/*  APIT.H -    Interface for creating and destroying time-sensitive interrupt hooks for the APIT
 *               - 1ns granularity
 *
 *  Author: Crupette
 * */

#ifndef MODULE_APIT_H
#define MODULE_APIT_H 1

#include "kernel/types.h"

typedef struct clock_hook {
    void (*hook)(void);
    uint32_t ns;
    int32_t ns_left;
} clock_hook_t;

/*  Adds a clock hook to be called every [ns] nanoseconds
 *  name:   Name to identify hook
 *  hook:   Function to call when time comes
 *  ns:     Nano-seconds until next call
 *  r:      Pointer to the clock hook
 * */
clock_hook_t *apit_add_clock(void (*hook)(void), uint32_t ns);

/*  Removes the requested clock hook
 *  hook:   Pointer to hook to remove
 * */
void apit_remove_clock(clock_hook_t *hook);

void apit_adjust_clock(clock_hook_t *hook);

#endif
