/*  PIT.H - Allows other modules to interface with and use the PIT
 *
 *  Author: Crupette
 * */

#ifndef MODULE_PIT_H
#define MODULE_PIT_H 1

#include "kernel/types.h"

#define PIT_BASE_HZ 1193182     //1.193182MHz

#define PIT_CH0_DATA    0x40
#define PIT_CH1_DATA    0x41
#define PIT_CH2_DATA    0x42
#define PIT_MCR         0x43

typedef struct pit_mcr {
    union {
        struct {
            uint8_t bcd     : 1;    //Binary or 4-bit BCD
            uint8_t op      : 3;    //Operating mode
            uint8_t access  : 2;    //Access mode
            uint8_t chnl    : 2;    //Channel
        };
        uint8_t data;
    };
} pit_mcr_t;

#define PIT_OP_TERMINAL 0   //Interrupt on terminal count
#define PIT_OP_ONESHOT  1   //Hardware re-triggerable one-shot
#define PIT_OP_RATE     2   //Rate generator
#define PIT_OP_SQWAVE   3   //Square wave generator
#define PIT_OP_SSTROBE  4   //Software-triggered strobe
#define PIT_OP_HSTROBE  5   //Hardware-triggered strobe

/*  Gets the current pit timer count (for mode 0)
 *  r:  PIT count
 * */
uint16_t pit_read_count(void);

/*  Sets the pit timer count (for mode 0)
 *  rel:    Reload value
 * */
void pit_set_count(uint16_t rel);

/*  Sets the mode of the PIT
 *  mode:   Mode to set it to
 * */
void pit_set_mode(uint8_t mode);

#endif
