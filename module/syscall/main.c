#include "api/syscall.h"
#include "api/message.h"

#include "module/user.h"
#include "module/interrupt.h"
#include "module/heap.h"
#include "module/datatype/list.h"

#include "kernel/print.h"
#include "kernel/string.h"
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

/*  Requests a buffer for IPC between processes
 *  bufsz:  Buffer's requested size:
 *  msgbuf: Pointer to message structure wanting to be filled
 *  ret:    Success
 * */
static int syscall_msg_req(size_t bufsz, message_t *msgbuf){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if(msgbuf == 0) return -1;  //TODO: EINVAL
    if(bufsz > 0xFF000) return -2; //TODO: ETOLARGE
    bufsz = ((bufsz + 0xFFF) / 0x1000) * 0x1000;

    msgbuf->src = utsk->pid;
    msgbuf->dat = (void*)0x1000;
    msgbuf->kdat = kalloc_a(bufsz, 0x1000);

    for(size_t i = 0; i < bufsz; i += 0x1000){
        virtual_allocator->remappg(
                (void*)virtual_allocator->getphys((uintptr_t)msgbuf->kdat + i),
                (void*)(i + (uintptr_t)msgbuf->dat),
                0x7);
    }

    return 0;
}
/*  Frees a buffer allocated by msg_req
 *  msg:    Message buffer to free
 *  r:      Success
 * */

static int syscall_msg_free(message_t *msg){
    if(msg == 0) return -1; //TODO: EINVAL
    if(msg->kdat == 0) return -1;

    kfree(msg->kdat);

    return 0;
}

/*  Sends a message to the requested pid
 *  msg:    Pointer to the message structure
 *  pid:    Recipient
 *  r:      Success
 * */
static int syscall_msg_send(message_t *msg, uint32_t pid){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    if(msg == 0) return -1; //TODO: EINVAL
    if(msg->msgsz >= 0xFF000) return -2; //TODO: ETOLARGE

    msg->src = utsk->pid;
    msg->dst = pid;

    message_t *kmsg = kalloc(sizeof(message_t));
    memcpy(kmsg, msg, sizeof(message_t));

    vga_printf("message: %p has %i bytes to send\n",
            msg->kdat, msg->msgsz);

    for(size_t i = 0; i < kmsg->msgsz; i += 0x1000){
        virtual_allocator->remappg((void*)0, (void*)(i + kmsg->dat), 0);
    }

    message_async_send(kmsg);
        
    return 0;
}

/*  Gets a message from the current tasks message queue
 *  msg:    Buffer to fill
 *  r:      Success
 * */
static int syscall_msg_recv(message_t *msg){
    if(msg == 0) return -1; //TODO: EINVAL   

    message_t *kmsg = message_async_recv();

    vga_printf("message: %p has %i bytes to recieve\n",
            kmsg->kdat, kmsg->msgsz);

    for(size_t i = 0; i < kmsg->msgsz; i += 0x1000){
        virtual_allocator->remappg(
                (void*)virtual_allocator->getphys((uintptr_t)kmsg->kdat + i),
                (void*)(i + (uintptr_t)kmsg->dat),
                0x7);
    }
   
    memcpy(msg, kmsg, sizeof(message_t));
    kfree(kmsg);

    return 0;
}

/*  Polls for a sent message.
 *  msg:    Buffer to fill
 *  r:      Success
 * */
static int syscall_msg_poll(message_t *msg){
    if(msg == 0) return -1; //TODO: EINVAL   

    message_t *kmsg = message_async_recv_poll();
    if(kmsg == 0) return -2;    //TODO: ENORESPONSE
    for(size_t i = 0; i < kmsg->msgsz; i += 0x1000){
        virtual_allocator->remappg(
                (void*)virtual_allocator->getphys((uintptr_t)kmsg->kdat + i),
                (void*)(i + (uintptr_t)kmsg->dat),
                0x5);
    }

    memcpy(msg, kmsg, sizeof(message_t));
    kfree(kmsg);

    return 0;
}

static int syscall_spawn(initrd_file_t *file, char *stk, size_t stksz){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    user_task_t *ntsk = user_spawn(file, stk, stksz, utsk->perms);
    if(ntsk != 0) return ntsk->pid;
    return -1;
}

static int syscall_fork(){
    int i = user_fork();
    return i;
}

static int syscall_exec(initrd_file_t *file, char *stk, size_t stksz){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;
    if(file == 0) return -1;

    return user_spawn(file, stk, stksz, utsk->perms) != 0;   
}

static int syscall_initrd_getfc(void){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_IRD) == 0) return -1; //TODO: ENOPERM
    uint32_t r = 0;
    initrd_get_files(&r);

    return r;
}

static int syscall_initrd_getf(initrd_file_t *buf, const char *name){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_IRD) == 0) return -1; //TODO: ENOPERM
    uint32_t fc = 0;
    uint32_t fi;
    initrd_file_t *files = initrd_get_files(&fc);

    for(fi = 0; fi < fc; fi++){
        if(strcmp(files[fi].name, name) == 0) break;
    }

    if(fi >= fc) return -2; //TODO: ENOTFOUND
    if(buf == 0) return -3; //TODO: EINVAL
    memcpy(buf, &files[fi], sizeof(initrd_file_t));

    return fi;
}

static int syscall_initrd_read(char *buf, uint32_t i, uint32_t base, uint32_t len){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_IRD) == 0) return -1; //TODO: ENOPERM
    uint32_t fc = 0;
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

    if((uintptr_t)virt >= 0xD8000000) return -2; //TODO: EOOB

    //vga_printf("Mapping page %p w/ flags %x\n",
    //        virt, flgs);
    return (int)virtual_allocator->mappg(virt, phys, flgs).entry;
}

static int syscall_allocpgs(void *virt, size_t cnt, uint32_t flgs){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_MEM) == 0) return -1; //TODO: ENOPERM
    if((uintptr_t)virt >= 0xD8000000) return -2; //TODO: EOOB

    //vga_printf("Allocating %i bytes from %p w/ flags %x\n",
    //        cnt, virt, flgs);
    return (int)virtual_allocator->allocpgs(virt, cnt, flgs);
}

static int syscall_freepgs(void *virt, size_t cnt){
    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;   

    if((utsk->perms & USER_PERM_MEM) == 0) return -1; //TODO: ENOPERM

    if((uintptr_t)virt >= 0xD8000000) return -2; //TODO: EOOB

    //vga_printf("Freeing %i bytes from %p\n",
    //        cnt, virt);
    virtual_allocator->freepgs(virt, cnt);
    return 0;
}

static void *scalls[] = {
    &syscall_print,

    &syscall_getperms,
    &syscall_getpid,

    &syscall_reqio,
    &syscall_blkio,
    &syscall_msg_req,
    &syscall_msg_free,
    &syscall_msg_send,
    &syscall_msg_recv,
    &syscall_msg_poll,

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

    //vga_printf("\033[37mTask %i syscall %i : %x, %x, %x, %x, %x %x\033[97m\n",
    //        utsk->pid, r->num, r->arg1, r->arg2, r->arg3, r->arg4, r->arg5, r->eip);

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
module_depends(heap);
module_depends(message);
module_depends(irq);
module_depends(idt);
