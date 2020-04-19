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

static void _user_second_half(uintptr_t entry, uintptr_t usrstkdata, uintptr_t usrstksize){
    asm volatile("add ebp, 4");

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
    
    asm volatile("add ebp, 4");

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

    user_task_t *utsk = kalloc(sizeof(user_task_t));
    utsk->pid = next_pid++;
    utsk->perms = perms;
    utsk->task = task_newtask(_user_spawn_second_half, kstk_top);
    
    list_push(utsk_list, utsk);
    return utsk;
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
