#include "module/timer.h"
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
#include "kernel/lock.h"

list_t *task_order = 0;
clock_hook_t *clock = 0;

list_node_t *current_task = 0;
list_node_t *next_task = 0;

extern DECLARE_LOCK(vga_op_lock);

#define TIME_SLICE_MAX 5.f

static void task_switch(){
    current_task = current_task->next;
    task_t *curr = current_task->data;

    uintptr_t esp, ebp, eip;
    eip = curr->ip;
    esp = curr->ksp;
    ebp = curr->kbp;

    //vga_printf("tp %p, %p, %p\n", esp, ebp, eip);

    virtual_allocator->swpdir(curr->dir);
    update_kstack(curr->kstack_top);

    asm volatile("mov ebx, %0; \
                  mov esp, %1; \
                  mov ebp, %2; \
                  mov eax, 0; \
                  jmp ebx":: "r"(eip), "r"(esp), "r"(ebp)
                  : "ebx", "esp", "eax");
}

//Forks the current process and jumps to func
task_t *task_newtask(void (*func)(void), uintptr_t stk){
    task_t *task = kalloc(sizeof(task_t));
    task->tslice = TIME_SLICE_MAX;
    task->priority = 0;
    task->ksp = stk;
    task->kstack_top = task->ksp;
    task->ip = func;
    task->new = true;
    task->dir = virtual_allocator->clonedir(virtual_allocator->currentDirectory);

    list_push(task_order, task);
    return task;
}

static void tasking_tick(void){
    if(task_order->head == 0) return;
    if(current_task->next == current_task) return;

    uint32_t esp, ebp, eip;
    asm volatile("mov %0, esp": "=r"(esp));
    asm volatile("mov %0, ebp": "=r"(ebp));
    
    extern uintptr_t read_eip();
    eip = read_eip();

    if(eip == 0){
        //vga_printf("Done\n");
        //Task switch is done
        return;
    }

    UNLOCK(vga_op_lock);
    //vga_printf("Switching from %p, %p, %p ", esp, ebp, eip);

    task_t *curr = current_task->data;
    curr->ksp = esp;
    curr->kbp = ebp;
    curr->ip = eip;

    task_switch();
}

extern uintptr_t stack_bottom;
extern uintptr_t stack_top;

int tasking_init(){
    task_order = new_round_list();

    //Bootstrap task
    task_t *bstask = kalloc(sizeof(task_t));
    bstask->tslice = TIME_SLICE_MAX;
    bstask->priority = 0;
    bstask->ksp = stack_top;
    bstask->dir = virtual_allocator->currentDirectory;

    list_push(task_order, bstask);
    current_task = task_order->head;

    clock = timer_add_clock(tasking_tick, TIME_SLICE_MAX);

    return 0;
}

int tasking_fini(){
    return 0;
}

module_name(tasking);

module_load(tasking_init);
module_unload(tasking_fini);

module_depends(gdt);
module_depends(timer);
module_depends(irq);
module_depends(heap);
module_depends(dthelper);
