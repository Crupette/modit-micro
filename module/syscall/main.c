#include "api/syscall.h"
#include "module/user.h"
#include "module/interrupt.h"
#include "module/heap.h"
#include "module/datatype/list.h"

#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/initrd.h"
#include "kernel/lock.h"

extern DECLARE_LOCK(vga_op_lock);
extern list_node_t *current_task;

static int syscall_print(char *msg){
    vga_printf("%s", msg);
}

static int syscall_getperms(){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    return utsk->perms;
}

static int syscall_getpid(){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    return utsk->pid;
}

static int syscall_reqio(uint16_t base, uint16_t len){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct; 

    if((int)base + (int)len > 0xFF) return -2;  //TODO: EOOB
    if(utsk->perms & USER_PERM_IO == 0) return -1; //TODO: ENOPERM

    for(int i = base; i < base + len; i++){
        ctsk->iobm[i / 8] &= ~(1 << (i % 8));
    }
    return 0;
}

static int syscall_blkio(uint16_t base, uint16_t len){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    if((int)base + (int)len > 0xFF) return -2;  //TODO: EOOB
    if(utsk->perms & USER_PERM_IO == 0) return -1;  //TODO: ENOPERM

    for(int i = base; i < base + len; i++){
        ctsk->iobm[i / 8] |= (1 << (i % 8));
    }
    return 0;
}

static int syscall_fork(){
    return user_fork();
}

static void *scalls[] = {
    &syscall_print,
    &syscall_getperms,
    &syscall_getpid,
    &syscall_reqio,
    &syscall_blkio,
    0,
    &syscall_fork,
};

void syscall_handler(syscall_state_t *r){
    extern list_node_t *current_task;

    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    vga_printf("\033[37mTask %i syscall %i : %x, %x, %x, %x, %x\033[97m\n",
            utsk->pid, r->num, r->arg1, r->arg2, r->arg3, r->arg4, r->arg5);

    memcpy(&utsk->state, r, sizeof(syscall_state_t));

    if(r->num >= SYSCALLS_NUM) return;

    int  ret = 0;
    int (*sc)(uint32_t, ...) = scalls[r->num];
    if(sc == 0) return;

    r->num = sc(r->arg1, r->arg2, r->arg3, r->arg4, r->arg5);
}

int syscall_init(){
    extern void isr_77_syscall();
    idt_createEntry(0x4D, isr_77_syscall, 0x08, 0x8E | 0x60);
    return 0;
}

int syscall_fini(){
    return 0;
}

module_name(syscall);

module_load(syscall_init);
module_unload(syscall_fini);

module_depends(tasking);
module_depends(user);
module_depends(irq);
module_depends(idt);
