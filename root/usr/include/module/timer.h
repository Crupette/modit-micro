/*  TIMER.H -    Interface for creating and destroying time-sensitive interrupt hooks
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
clock_hook_t *timer_add_clock(void (*hook)(void), uint32_t ns);

/*  Removes the requested clock hook
 *  hook:   Pointer to hook to remove
 * */
void timer_remove_clock(clock_hook_t *hook);

/*  Forces the timer to acknowlege changes in the given clocks interrupt frequency
 *  hook:   Clock to change - the ns value should be different than before
 * */
void timer_adjust_clock(clock_hook_t *hook);

/*  Reads the time remaining in the clock
 *  r:  Time remaining in arbitrary time units
 * */
uint32_t timer_read(void);

/*  Sets the time remaining in the clock
 *  count:  Time remaining in arbitrary time units
 * */
void timer_set(uint32_t count);

/*  Acknowleges the interrupt for the clock
 * */
void timer_ack(void);

#endif
