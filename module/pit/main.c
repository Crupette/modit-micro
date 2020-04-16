#include "module/pit.h"
#include "module/interrupt.h"

#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/io.h"

uint16_t pit_read_count(void){
    outb(PIT_MCR, 0);
    uint16_t r = 0;
    r = inb(PIT_CH0_DATA);
    r |= (((uint16_t)inb(PIT_CH0_DATA)) << 8);
    return r;
}

void pit_set_count(uint16_t rel){
    outb(PIT_CH0_DATA, rel & 0xFF);
    rel >> 8;
    outb(PIT_CH0_DATA, rel & 0xFF);
}

void pit_set_mode(uint8_t mode){
    outb(PIT_MCR, 0x30 | (mode << 1));
}

int pit_init(){
    disable_interrupts();

    //Default to 1/100s pulse
    int div = PIT_BASE_HZ / 100;
    outb(PIT_CH0_DATA, div & 0xFF);
    outb(PIT_CH0_DATA, div >> 8);

    enable_interrupts();
    return 0;
}

int pit_fini(){
    return 0;
}

module_name(pit);

module_load(pit_init);
module_unload(pit_fini);

module_depends(irq);
