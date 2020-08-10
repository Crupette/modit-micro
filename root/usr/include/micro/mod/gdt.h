#ifndef MODULE_GDT_H
#define MODULE_GDT_H 1

#include <stdint.h>
#include <stddef.h>

typedef struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    union {
        uint8_t access;
        struct {
            uint8_t accessed    : 1;
            uint8_t rw          : 1;
            uint8_t dc          : 1;
            uint8_t exec        : 1;
            uint8_t s           : 1;
            uint8_t priv        : 2;
            uint8_t present     : 1;
        }__attribute__((packed));
    };
    union {
        uint8_t flags;
        struct {
            uint8_t limit_high  : 4;
            uint8_t zero        : 1;
            uint8_t resv        : 1;
            uint8_t opsz        : 1;
            uint8_t gran        : 1;
        }__attribute__((packed));
    };
    uint8_t base_high;
}__attribute__((packed)) gdt_entry_t;

struct iobmap {
    uint8_t bm[8192];
};

struct tss32 {
    uint32_t lnk;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldtr;
    uint16_t resv;
    uint16_t iopb_off;
}__attribute__((packed));

struct tss64 {
    uint32_t resv;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t resv1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t resv2;
    uint16_t resv3;
    uint16_t iopb_off;
}__attribute__((packed));

#if defined(BITS_64)
typedef struct tss64 tss_entry_base;
#elif defined(BITS_32)
typedef struct tss32 tss_entry_base;
#endif

typedef struct tss_entry {
    tss_entry_base base;
    struct iobmap iobmap;
}__attribute__((pakced)) tss_entry_t;

typedef struct gdt_desc {
    uint16_t size;
    uintptr_t addr;
} __attribute__((packed)) gdt_desc_t;

void add_gdt(uint8_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flg);

void add_tss(uint8_t i);

void update_iobm(uint8_t *bm);

void update_kstk(uintptr_t sp);

#endif
