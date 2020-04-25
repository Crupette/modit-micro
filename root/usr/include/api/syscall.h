/*  SYSCALL.H - Provides low level device access and manipulation for the userspace to control
 *
 *  Author: Crupette
 * */

#ifndef MODULE_SYSCALL_H
#define MODULE_SYSCALL_H 1

typedef struct syscall_state {
    unsigned int num;
    unsigned int arg1, arg2, arg3, arg4, arg5;
    unsigned int ebp, ds, es, fs, gs;
    unsigned int eip, cs, eflags, usresp, ss;
} syscall_state_t;

enum {
    SYSCALL_PRINT = 0,  //Debug syscall

    SYSCALL_GETPERMS,   //Info Requests
    SYSCALL_GETPID,

    SYSCALL_REQIO,      //IO
    SYSCALL_BLKIO,

    SYSCALL_SPAWN,      //Proc
    SYSCALL_FORK,
    SYSCALL_EXEC,

    SYSCALL_INITRD_GETFC,   //Initrd
    SYSCALL_INITRD_GETF,
    SYSCALL_INITRD_READ,

    SYSCALL_RESVPG,
    SYSCALL_MAPPG,      //Raw Memory Access
    SYSCALL_ALLOCPGS,
    SYSCALL_FREEPGS,
    
    SYSCALLS_NUM
};

#define USER_PERM_IO 0x1
#define USER_PERM_IRD 0x2
#define USER_PERM_MEM 0x4

#define DECL_SYSCALL0(fn) int syscall_##fn();
#define DECL_SYSCALL1(fn, p1) int syscall_##fn(p1);
#define DECL_SYSCALL2(fn, p1, p2) int syscall_##fn(p1, p2)
#define DECL_SYSCALL3(fn, p1, p2, p3) int syscall_##fn(p1, p2, p3)
#define DECL_SYSCALL4(fn, p1, p2, p3, p4) int syscall_##fn(p1, p2, p3, p4)
#define DECL_SYSCALL5(fn, p1, p2, p3, p4, p5) int syscall_##fn(p1, p2, p3, p4, p5)

#define DEFN_SYSCALL0(fn, num) \
int syscall_##fn() { \
	int a; \
	asm volatile("int $0x4D" : "=a" (a) : "0" (num)); \
	return a; \
}

#define DEFN_SYSCALL1(fn, num, P1) \
int syscall_##fn(P1 p1) { \
	int a; \
	asm volatile("int $0x4D" : "=a" (a) : "0" (num), "b" ((int)p1)); \
	return a; \
}

#define DEFN_SYSCALL2(fn, num, P1, P2) \
int syscall_##fn(P1 p1, P2 p2) { \
	int a; \
	asm volatile("int $0x4D" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2)); \
	return a; \
}

#define DEFN_SYSCALL3(fn, num, P1, P2, P3) \
int syscall_##fn(P1 p1, P2 p2, P3 p3) { \
	int a; \
	asm volatile("int $0x4D" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3)); \
	return a; \
}

#define DEFN_SYSCALL4(fn, num, P1, P2, P3, P4) \
int syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4) { \
	int a; \
	asm volatile("int $0x4D" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3), "S" ((int)p4)); \
	return a; \
}

#define DEFN_SYSCALL5(fn, num, P1, P2, P3, P4, P5) \
int syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) { \
	int a; \
	asm volatile("int $0x4D" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3), "S" ((int)p4), "D" ((int)p5)); \
	return a; \
}


#endif
