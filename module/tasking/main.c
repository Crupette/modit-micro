#include "module/apit.h"
#include "module/apic/apic.h"
#include "module/tasking.h"
#include "module/interrupt.h"
#include "module/datatype/list.h"
#include "module/heap.h"

#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/io.h"
#include "kernel/vgaterm.h"

list_t *task_order = 0;
clock_hook_t *clock = 0;

list_node_t *current_task = 0;

#define TIME_SLICE_MAX 100

void task_switch(task_t *curr, task_t *next){
    //Save previous EBP and ESP
    asm volatile("mov %0, ebp; \
                  nop": "=r"(curr->kbp));
    asm volatile("mov %0, esp; \
                  nop": "=r"(curr->ksp));
    
    if(next->dir != virtual_allocator->currentDirectory){
            curr->dir = virtual_allocator->swpdir(next->dir);
    }

    current_task = current_task->next;

    //Change clock to tick towards next task's timeslice
    clock->ns = clock->ns_left = (next->tslice);
    apit_adjust_clock(clock);

    if(next->new == false){
        //Copy over base and stack pointer
        asm volatile("mov eax, %0; \
                      mov ebx, %1; \
                      mov ebp, ebx; \
                      mov esp, eax; \
                      nop":: "r"(next->ksp), "r"(next->kbp));
    }else{
        next->new = false;
        //Make sure interrupts are acknowleged, and return from interrupt
        apic_ack();
        asm volatile("mov esp, %0;\
                pop gs; \
                pop fs; \
                pop es; \
                pop ds; \
                popa; \
                add esp, 8; \
                iret" :: "r"(next->ksp));
    }
}

void task_stack_add(task_t *task, void *data, size_t size){
    task->ksp = (uintptr_t*)((uintptr_t)task->ksp - size);
    memcpy(task->ksp, data, size);
}

void task_newtask(void (*func)(void)){
    task_t *task = kalloc(sizeof(task_t));
    task->tslice = TIME_SLICE_MAX;
    task->priority = 0;
    task->ksp = (kalloc(0x4000) + 0x3FFF);  //Kernel stack : 4KiB
    task->new = true;
    task->dir = virtual_allocator->clonedir(virtual_allocator->currentDirectory);

    //Save states for new task execution
    interrupt_state_t newstate = { 0 };
    newstate.eax = newstate.ebx = newstate.ecx = newstate.edx = newstate.esi = newstate.edi = 0;
    newstate.eip = (uintptr_t)func;
    newstate.esp = newstate.ebp = (uintptr_t)task->ksp;
    newstate.ds = newstate.es = newstate.es = newstate.fs = newstate.gs = newstate.ss = 0x10;
    newstate.cs = 0x8;
    newstate.eflags = 0 | (1 << 9); //INT_ENABLE

    task_stack_add(task, &newstate, sizeof(interrupt_state_t));

    list_push(task_order, task);
}

void tasking_tick(void){
    if(task_order->head == 0) return;

    task_switch(current_task->data, current_task->next->data);
}

extern uintptr_t stack_bottom;
extern uintptr_t stack_top;

void task_loop_b(void){
    while(true){
        vgaterm_putc('A');
    }
}

void task_loop_c(void){
    while(true){
        vgaterm_putc('B');
    }
}

int tasking_init(){
    task_order = new_round_list();

    //Bootstrap task
    task_t *bstask = kalloc(sizeof(task_t));
    bstask->tslice = TIME_SLICE_MAX;
    bstask->priority = 0;
    bstask->ksp = &stack_top;
    bstask->dir = virtual_allocator->currentDirectory;

    list_push(task_order, bstask);
    current_task = task_order->head;

    task_newtask(task_loop_b);
    task_newtask(task_loop_c);

    clock = apit_add_clock(tasking_tick, TIME_SLICE_MAX);

    //Temporary idle task for the test
    while(true){
        asm("hlt");
    }

    return 0;
}

int tasking_fini(){
    return 0;
}

module_name(tasking);

module_load(tasking_init);
module_unload(tasking_fini);

module_depends(timer);
module_depends(apic);
module_depends(irq);
module_depends(heap);
module_depends(dthelper);
