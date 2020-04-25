#include "module/user.h"
#include "module/interrupt.h"
#include "module/datatype/list.h"
#include "module/heap.h"

#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/io.h"
#include "kernel/vgaterm.h"
#include "kernel/lock.h"

uint32_t next_pid = 0;
list_t *utsk_list = 0;
DECLARE_LOCK(utsk_lock) = 0;

extern list_node_t *current_task;

static void _user_second_half(uintptr_t entry, uintptr_t usrstkdata, uintptr_t usrstksize){
    uintptr_t ustk = virtual_allocator->allocpgs(0xCFFFC000, 0x4000, 0x7);
    ustk += 0x4000;

    if(usrstkdata == 0 || usrstksize == 0){
        ustk -= usrstksize;
        memcpy((void*)ustk, (void*)usrstkdata, usrstksize);
    }

    extern void usr_switch(uintptr_t entry, uintptr_t stk);
    usr_switch(entry, ustk);

    //task should not reach this point
    while(true) asm("hlt");
}

static void _user_spawn_second_half(
        initrd_file_t *file, 
        uintptr_t usrstkdata, 
        uintptr_t usrstksize){
    
    asm volatile("sub ebp, 4");

    uintptr_t uentry = modit_elf_load(file);
    uintptr_t ustk = virtual_allocator->allocpgs(0xCFFFC000, 0x4000, 0x7);
    ustk += 0x4000;

    if(usrstkdata != 0 && usrstksize != 0){
        ustk -= usrstksize;
        memcpy((void*)ustk, (void*)usrstkdata, usrstksize);
    }

    extern void usr_switch(uintptr_t entry, uintptr_t stk);
    usr_switch(uentry, ustk);

    //task should not reach this point
    while(true) asm("hlt");
}

user_task_t *user_spawn(
        initrd_file_t *file, 
        void *usrstkdata, 
        uintptr_t usrstksize, 
        uint32_t perms){

    char *kstack = kalloc(0x2000);  //2KiB kernel stack
    memset(kstack, 0, 0x2000);

    char *newstkdat = kalloc(usrstksize);
    memcpy(newstkdat, usrstkdata, usrstksize);

    //Add arguments for second half execution
    char *kstk_top = kstack + 0x2000;
    ((uintptr_t*)kstk_top)[-3] = file;
    ((uintptr_t*)kstk_top)[-2] = (uintptr_t)newstkdat;
    ((uintptr_t*)kstk_top)[-1] = usrstksize;

    kstk_top -= 12;

    LOCK(utsk_lock);
    tasking_disable();

    user_task_t *utsk = kalloc(sizeof(user_task_t));
    memset(utsk, 0, sizeof(user_task_t));
    utsk->pid = next_pid++;
    utsk->perms = perms;
    utsk->task = task_newtask(_user_spawn_second_half, kstk_top);
    utsk->task->parent_struct = utsk;
    
    list_push(utsk_list, utsk);

    tasking_enable();
    UNLOCK(utsk_lock);

    return utsk;
}

#define STK_PUSH(sp, data, size) \
    sp -= size; \
    memcpy((void*)sp, data, size)

user_task_t *user_fork(){
    task_t *ctsk = current_task->data;
    user_task_t *putsk = ctsk->parent_struct;

    LOCK(utsk_lock);
    user_task_t *utsk = kalloc(sizeof(user_task_t));
    memset(utsk, 0, sizeof(user_task_t));
    utsk->pid = next_pid++;
    utsk->perms = putsk->perms;

    tasking_disable();  //We don't want to mangle the new task
    
    extern void usr_ret();
    utsk->task = task_newtask(usr_ret, kalloc(0x2000) + 0x2000);
    utsk->task->parent_struct = utsk;
    utsk->state.num = 0;

    STK_PUSH(utsk->task->ksp, &putsk->state, sizeof(syscall_state_t));
    
    tasking_enable();

    list_push(utsk_list, utsk);
    UNLOCK(utsk_lock);

    return utsk;
}

void user_exec(initrd_file_t *file, void *ustkdata, uintptr_t ustksize, uint32_t perms){
    uintptr_t elf_entry = modit_elf_load(file);
    uintptr_t ustk = 0xD0000000;

    if(ustkdata != 0 && ustksize != 0){
        ustk -= ustksize;
        memcpy((void*)ustk, (void*)ustkdata, ustksize);
    }

    extern void usr_switch(uintptr_t entry, uintptr_t stk);
    usr_switch(elf_entry, ustk);

    while(true) asm("hlt");
}

void user_block(uint32_t id){
    vga_printf("Blocking %i\n", id);
    for(list_node_t *node = utsk_list->head; node; node = node->next){
        user_task_t *utsk = node->data;
        if(utsk->pid == id){
            utsk->task->blocked = true;
            return;
        }
    }
}

void user_awake(uint32_t id){
    vga_printf("Unblocking %i\n", id);
    for(list_node_t *node = utsk_list->head; node; node = node->next){
        user_task_t *utsk = node->data;
        if(utsk->pid == id){
            utsk->task->blocked = false;
            return;
        }
    }
}

bool user_isblocked(uint32_t id){
     for(list_node_t *node = utsk_list->head; node; node = node->next){
        user_task_t *utsk = node->data;
        if(utsk->pid == id){
            return utsk->task->blocked;
        }
    }
    return false; 
}


int user_init(){
    utsk_list = new_list();
    return 0;
}

int user_fini(){
    return 0;
}

module_name(user);

module_load(user_init);
module_unload(user_fini);

module_depends(tasking);
module_depends(heap);
module_depends(dthelper);
module_depends(gdt);
