#include "module/apic/apic.h"
#include "module/timer.h"
#include "module/interrupt.h"
#include "module/heap.h"

#include "kernel/modloader.h"
#include "kernel/logging.h"
#include "kernel/io.h"

#define APIC_LVT_TMR    0x320
#define APIC_TMRINITCNT 0x380
#define APIC_TMRCURRCNT 0x390
#define APIC_TMRDIV     0x3E0

static uint32_t base_freq = 0;
static uint32_t freq_1s = 0;
static uint32_t freq_1ns = 0;

uint32_t timer_shortest = 0;
clock_hook_t *shortest_clock = 0;

list_t *clock_hooks = 0;

bool apic = false;

uint32_t timer_read(void){
    if(apic) return apic_read(APIC_TMRINITCNT);
    vga_printf("No APIC?\n");
    return 0;
}

void timer_set(uint32_t count){
    if(apic){
        apic_write(APIC_TMRINITCNT, count);
        apic_write(APIC_TMRCURRCNT, count);
    }
}

void timer_ack(void){
    if(apic) apic_ack();
}

clock_hook_t *timer_add_clock(void (*hook)(void), uint32_t ns){
    clock_hook_t *clock = kalloc(sizeof(clock_hook_t));
    clock->hook = hook;
    clock->ns = ns;
    clock->ns_left = ns;
    list_push(clock_hooks, clock);

    uint32_t ns_passed = timer_read() / freq_1ns;

    //If it asks for a shorter clock time, we must change the APIT to accomidate
    if(timer_shortest - ns_passed > ns){
        timer_set(ns * freq_1ns);
    }
    if(timer_shortest > ns || shortest_clock == 0){
        if(shortest_clock == 0){
            timer_set(ns * freq_1ns);
        }
        timer_shortest = ns;
        shortest_clock = clock;
    }
    return clock;
}

void timer_adjust_clock(clock_hook_t *hook){
    if(hook == 0) return;

    uint32_t ns_passed = timer_read() / freq_1ns;

    if(timer_shortest - ns_passed < hook->ns){
        timer_set(hook->ns * freq_1ns);
    }
    if(hook->ns < timer_shortest){
        shortest_clock = hook;
        timer_shortest = hook->ns;
    }
}

void timer_remove_clock(clock_hook_t *hook){
    if(hook == 0) return;
    list_remove(clock_hooks, hook);
    if(hook == shortest_clock){
        timer_shortest = 0xFFFFFFFF;
        shortest_clock = 0;
        for(list_node_t *node = clock_hooks->head; node; node = node->next){
            clock_hook_t *clock = node->data;
            if(clock->ns <= timer_shortest){
                timer_shortest = clock->ns;
                shortest_clock = clock;
            }
        }
    }
    kfree(hook);
}

list_node_t *hook_node = 0;
clock_hook_t *hook_clock = 0;

void timer_interrupt(interrupt_state_t *r){
    (void)r;
    uint32_t ns_passed = timer_read() / freq_1ns;
    timer_set(timer_shortest * freq_1ns);

    for(hook_node = clock_hooks->head; hook_node; hook_node = hook_node->next){
        hook_clock = hook_node->data;
        hook_clock->ns_left -= ns_passed;

        if(hook_clock->ns_left <= 0){
            hook_clock->hook();
            hook_clock->ns_left += hook_clock->ns;
        }
    }
    timer_ack();
}

void one_tick(void){
}

int timer_init(){
    //APIT needs to be calibrated
    if(apic_enabled){
        log_printf(LOG_DEBUG, "Using the APIC\n");
        apic_write(APIC_TMRDIV, 0x3);
        apic_write(APIC_LVT_TMR, 32);
        
        //PIT Channel 2 one shot : wait 1/100 sec
        outb(0x61, (inb(0x61) & 0xFD) | 1);
        outb(0x43, 0xB2);
        
        //PIT 100hz
        outb(0x42, 0x9b);
        inb(0x60);
        outb(0x42, 0x2e);
        
        //Reset PIT counter
        outb(0x61, (inb(0x61) & 0xFE) | 1);
        apic_write(APIC_TMRINITCNT, 0xFFFFFFFF);
        
        //Wait for PIT
        while(inb(0x61) & 0x20) {}
        
        //Stop APIC
        apic_write(APIC_LVT_TMR, 0x10000);

        uint32_t apic_timer_count = 0xFFFFFFFF - apic_read(APIC_TMRCURRCNT);
        base_freq = apic_timer_count;
        
        apic_timer_count *= 100;
        
        freq_1s = apic_timer_count;
        freq_1ns = apic_timer_count / 100000;
        
        apic_write(APIC_TMRDIV, 0x3);
        apic_write(APIC_LVT_TMR, 32);

        apic = true;
    }
    
    clock_hooks = new_list();

    irq_addHandler(0, timer_interrupt);
    timer_add_clock(one_tick, 100000);

    return 0;
}

int timer_fini(){
    return 0;
}

module_name(timer);

module_load(timer_init);
module_unload(timer_fini);

module_depends(apic);
module_depends(irq);
module_depends(heap);
module_depends(dthelper);
