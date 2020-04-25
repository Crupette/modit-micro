#include "api/syscall.h"
#include "module/user.h"
#include "module/interrupt.h"
#include "module/heap.h"
#include "module/datatype/list.h"

#include "kernel/io.h"
#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/initrd.h"
#include "kernel/lock.h"

extern DECLARE_LOCK(vga_op_lock);
extern list_node_t *current_task;

static int syscall_print(char *msg){
    vga_printf("%s", msg);
    return 0;
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
    if((utsk->perms & USER_PERM_IO) == 0) return -1; //TODO: ENOPERM

    for(int i = base; i < base + len; i++){
        ctsk->iobm[i / 8] &= ~(1 << (i % 8));
    }
    return 0;
}

static int syscall_blkio(uint16_t base, uint16_t len){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    if((int)base + (int)len > 0xFF) return -2;  //TODO: EOOB
    if((utsk->perms & USER_PERM_IO) == 0) return -1;  //TODO: ENOPERM

    for(int i = base; i < base + len; i++){
        ctsk->iobm[i / 8] |= (1 << (i % 8));
    }
    return 0;
}

static int syscall_spawn(initrd_file_t *file, char *stk, size_t stksz){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    return user_spawn(file, stk, stksz, utsk->perms) != 0;
}

static int syscall_fork(){
    return user_fork() != 0;
}

static int syscall_exec(initrd_file_t *file, char *stk, size_t stksz){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    return user_spawn(file, stk, stksz, utsk->perms) != 0;   
}

static int syscall_initrd_getfc(void){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_IRD) == 0) return -1; //TODO: ENOPERM
    size_t r = 0;
    initrd_get_files(&r);

    return r;
}

static int syscall_initrd_getf(initrd_file_t *buf, uint32_t i){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_IRD) == 0) return -1; //TODO: ENOPERM
    size_t fc = 0;
    initrd_file_t *files = initrd_get_files(&fc);

    if(i >= fc) return -2; //TODO: EOOB
    if(buf == 0) return -3; //TODO: EINVAL
    if(buf->name == 0) return -4; //TODO: EINVAL

    memcpy(buf->name, files[i].name, strlen(files[i].name));
    buf->size = files[i].size;

    return 0;
}

static int syscall_initrd_read(char *buf, uint32_t i, uint32_t base, uint32_t len){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_IRD) == 0) return -1; //TODO: ENOPERM
    size_t fc = 0;
    initrd_file_t *files = initrd_get_files(&fc);

    if(i >= fc) return -2; //TODO: EOOB
    if(base >= files[i].size) return 0;
    if(base + len >= files[i].size){
        len = files[i].size - base;
    }
    memcpy(buf, files[i].data_start + base, len);
    return len;
}

static int syscall_resvpg(void *phys){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_MEM) == 0) return -1; //TODO: ENOPERM
    physical_allocator->resvpg(phys);
    return 0;
}

static int syscall_mappg(void *virt, void *phys, uint32_t flgs){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_MEM) == 0) return -1; //TODO: ENOPERM

    if(virt >= 0xD8000000) return -2; //TODO: EOOB
    return virtual_allocator->mappg(virt, phys, flgs).entry;
}

static int syscall_allocpgs(void *virt, size_t cnt, uint32_t flgs){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_MEM) == 0) return -1; //TODO: ENOPERM

    return virtual_allocator->allocpgs(virt, cnt, flgs);
}

static int syscall_freepgs(void *virt, size_t cnt){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_MEM) == 0) return -1; //TODO: ENOPERM

    if(virt >= 0xD8000000) return -2; //TODO: EOOB
    virtual_allocator->freepgs(virt, cnt);
    return 0;
}

static void *scalls[] = {
    &syscall_print,
    &syscall_getperms,
    &syscall_getpid,
    &syscall_reqio,
    &syscall_blkio,
    &syscall_spawn,
    &syscall_fork,
    &syscall_exec,
    &syscall_initrd_getfc,
    &syscall_initrd_getf,
    &syscall_initrd_read,
    &syscall_resvpg,
    &syscall_mappg,
    &syscall_allocpgs,
    &syscall_freepgs,
};

void syscall_handler(syscall_state_t *r){
    extern list_node_t *current_task;
    if(r->num >= SYSCALLS_NUM) return;

    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    memcpy(&utsk->state, r, sizeof(syscall_state_t));

    int ret = 0;
    int (*sc)() = scalls[r->num];
    if(sc == 0){
        log_printf(LOG_WARNING, "Task %i called for illegal syscall %i\n",
                utsk->pid, r->num);
        r->num = -1;
        return;
    }

    //vga_printf("\033[37mTask %i syscall %i : %x, %x, %x, %x, %x\033[97m\n",
    //        utsk->pid, r->num, r->arg1, r->arg2, r->arg3, r->arg4, r->arg5);

    ret = sc(r->arg1, r->arg2, r->arg3, r->arg4, r->arg5);
    r->num = ret;
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
