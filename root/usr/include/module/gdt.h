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
			uint8_t accessed	: 1; //If the segment is accessed
			uint8_t rw		: 1; //Readable / Writable
			uint8_t dc		: 1; //Direction or conforming
			uint8_t exec		: 1; //Usage of segment (code / data vs system)
			
			uint8_t priv		: 2; //Privilege ring. 0-3
			uint8_t present		: 1; //If the segment is valid
		}__attribute__((packed));
	};
	union {
		uint8_t flags;
		struct {
			uint8_t limit_high	: 4;
			uint8_t zero		: 1; //Always Zero
			uint8_t reserved	: 1; //Always Zero, different for x64
			uint8_t opsize		: 1; //x64 identifier
			uint8_t granularity	: 1; //0 = 1 byte, 1 = 4kb
		}__attribute__((packed));
	};
	uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt_descriptor {
	uint16_t size;
	uintptr_t addr;
} __attribute__((packed)) gdt_descriptor_t;

/*	Creates a new GDT entry based on the information given
 *	base:	Address to begin translation
 *	limit:	Number of bytes to target
 *	access:	General purpose flags
 *	flags:	More specialized flags
 *	return:	New GDT entry
 * */
gdt_entry_t build_gdt(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

/*	Adds or changes a GDT entry from the table to one defined by [src]
 *	target:	Index in the table to place entry
 *	src:	Data to take from
 * */
void add_gdt(unsigned int target, gdt_entry_t src);

#endif
