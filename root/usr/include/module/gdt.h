/*  GDT.H - Module interface for modules to follow.
 *      If you want your GDT module to be interfaced with other modules, use this function
 *
 *  Authot: Crupette
 * */

#ifndef MOD_GDT_H
#define MOD_GDT_H

#include <stdint.h>
#include <stddef.h>

typedef struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    union {
        uint8_t access;
        struct {
            uint8_t accessed    : 1; //If the segment is accessed
            uint8_t rw      : 1; //Readable / Writable
            uint8_t dc      : 1; //Direction or conforming
            uint8_t exec        : 1; //Usage of segment (code / data vs system)
            uint8_t s       : 1; //Descriptor type
            uint8_t priv        : 2; //Privilege ring. 0-3
            uint8_t present     : 1; //If the segment is valid
        }__attribute__((packed));
    };
    union {
        uint8_t flags;
        struct {
            uint8_t limit_high  : 4;
            uint8_t zero        : 1; //Always Zero
            uint8_t reserved    : 1; //Always Zero, different for x64
            uint8_t opsize      : 1; //x64 identifier
            uint8_t granularity : 1; //0 = 1 byte, 1 = 4kb
        }__attribute__((packed));
    };
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

struct iobmap {
    uint8_t bm[8192];
};

struct tss_entry_base {
    uint32_t link;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs, ldtr;
    uint16_t resv;
    uint16_t iopb_off;
} __attribute__((packed));

typedef struct tss_entry {
    struct tss_entry_base base;
    struct iobmap iobmap;
} __attribute__((packed))tss_entry_t;


typedef struct gdt_descriptor {
    uint16_t size;
    uintptr_t addr;
} __attribute__((packed)) gdt_descriptor_t;

/*  Creates a new GDT entry based on the information given
 *  i:      Index to place entry
 *  base:   Address to begin translation
 *  limit:  Number of bytes to target
 *  access: General purpose flags
 *  flags:  More specialized flags
 * */
void add_gdt(uint8_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

/*  Creates a new GDT entry to house the TSS
 *  i:      Index to place entry
 * */
void add_tss(uint8_t i);

/*  Copies the bm passed into the tss iopbm
 *  bm: Bitmap to copy over
 * */
void update_iobm(uint8_t *bm);

/*  Sets the TSS kernel stack
 *  esp:    Stack pointer
 * */
void update_kstack(uint32_t esp);

#endif
