#include "module/cpu/apic.h"
#include "module/apit.h"
#include "module/interrupt.h"

#include "kernel/modloader.h"
#include "kernel/logging.h"
#include "kernel/io.h"

#define APIC_LVT_TMR    0x320
#define APIC_TMRINITCNT 0x380
#define APIC_TMRCURRCNT 0x390
#define APIC_TMRDIV     0x3E0

uint32_t apit_base_freq = 0;
uint32_t apit_freq_1s = 0;
uint32_t apit_freq_1ns = 0;

uint32_t apit_shortest = 0;
clock_hook_t *apit_shortest_clock = 0;

list_t *clock_hooks = 0;

void apit_add_clock(char *name, void (*hook)(void), uint32_t ns){
    uint32_t ns_passed = apic_read(APIC_TMRCURRCNT) / apit_freq_1ns;

    clock_hook_t *clock = kalloc(sizeof(clock_hook_t));
    clock->name = name;
    clock->hook = hook;
    clock->ns = ns;
    clock->ns_left = ns;
    list_push(clock_hooks, clock);

    //If it asks for a shorter clock time, we must change the APIT to accomidate
    if(apit_shortest - ns_passed < ns){
        apic_write(APIC_TMRDIV, 0x3);
        apic_write(APIC_TMRINITCNT, ns * apit_freq_1ns);
        apic_write(APIC_TMRCURRCNT, ns * apit_freq_1ns);
    }
    if(apit_shortest > ns || apit_shortest_clock == 0){
        if(apit_shortest_clock == 0){
            apic_write(APIC_TMRDIV, 0x3);
            apic_write(APIC_TMRINITCNT, ns * apit_freq_1ns);
            apic_write(APIC_TMRCURRCNT, ns * apit_freq_1ns);
        }
        apit_shortest = ns;
        apit_shortest_clock = clock;
    }
}

void apit_remove_clock(char *name){
    clock_hook_t *hook = 0;
    for(list_node_t *node = clock_hooks->head; node; node = node->next){
        clock_hook_t *clock = node->data;
        if(name == clock->name || strcmp(name, clock->name) == 0){
            hook = clock;
            break;
        }
    }

    if(hook == 0) return;
    if(hook == apit_shortest_clock){
        apit_shortest = 0xFFFFFFFF;
        apit_shortest_clock = 0;
        for(list_node_t *node = clock_hooks->head; node; node = node->next){
            clock_hook_t *clock = node->data;
            if(clock->ns <= apit_shortest){
                apit_shortest = clock->ns;
                apit_shortest_clock = clock;
            }
        }
    }
}

void apit_interrupt(interrupt_state_t *r){
    uint32_t ns_passed = apic_read(APIC_TMRINITCNT) / apit_freq_1ns;
    apic_write(APIC_TMRINITCNT, apit_shortest * apit_freq_1ns);

    for(list_node_t *node = clock_hooks->head; node; node = node->next){
        clock_hook_t *clock = node->data;
        clock->ns_left -= ns_passed;

        if(clock->ns_left <= 0){
            clock->hook();
            clock->ns_left += clock->ns;
        }
    }
}

void one_tick(void){
}

int apit_init(){
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
    apit_base_freq = apic_timer_count;

    apic_timer_count << 4;
    apic_timer_count *= 100;

    apit_freq_1s = apic_timer_count;
    apit_freq_1ns = apic_timer_count / 100000;

    apic_write(APIC_LVT_TMR, 32);
    apic_write(APIC_TMRDIV, 0x3);

    clock_hooks = new_list();

    irq_addHandler(0, apit_interrupt);
    apit_add_clock("1s-tick", one_tick, 100000);

    return 0;
}

int apit_fini(){
    return 0;
}

module_name(apit);

module_load(apit_init);
module_unload(apit_fini);

module_depends(cpu);
module_depends(irq);
module_depends(heap);
module_depends(dthelper);
