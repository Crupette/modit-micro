/*  APIT.H -    Interface for creating and destroying time-sensitive interrupt hooks for the APIT
 *               - 1ns granularity
 *
 *  Author: Crupette
 * */

#ifndef MODULE_APIT_H
#define MODULE_APIT_H 1

#include "kernel/types.h"

typedef struct clock_hook {
    char *name;
    void (*hook)(void);
    uint32_t ns;
    int32_t ns_left;
} clock_hook_t;

/*  Adds a clock hook to be called every [ns] nanoseconds
 *  name:   Name to identify hook
 *  hook:   Function to call when time comes
 *  ns:     Nano-seconds until next call
 * */
void apit_add_clock(char *name, void (*hook)(void), uint32_t ns);

/*  Removes the requested clock hook
 *  name:   Name of hook to remove
 * */
void apit_remove_clock(char *name);

#endif
