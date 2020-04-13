/*  APIC.H -    Structures and functions required for the kernel to utilize the APIC
 *
 *  Author: Cupette
 * */
#ifndef MODULE_CPU_APIC_H
#define MODULE_CPU_APIC_H 1

#include "module/cpu/acpi.h"
#include "module/datatype/list.h"

#include "kernel/types.h"

#define APIC_WRITE(i, value)    ((uint32_t volatile*)(apic_registry + i))[0] = value
#define APIC_READ(i)            *((uint32_t volatile*)(apic_registry + i))

#define IOAPIC_REDIR_TABLE(n) (0x10 + (2 * n))

typedef struct madt {
    acpi_sdt_hdr_t header;
    uint32_t local_apic_addr;
    uint32_t flags;
} madt_t;

typedef struct madt_entry_header {
    uint8_t type;
    uint8_t length;
} madt_entry_header_t;

typedef struct madt_apic_local_entry {
    madt_entry_header_t header;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} madt_apic_local_entry_t;

typedef union ioapic_redir_entry {
    struct {
        uint32_t vector     : 8;    //Interrupt vector raised
        uint32_t delivery   : 3;    //How interrupt will be sent to CPU's
        uint32_t dtype      : 1;    //0: Physical destination, 1: Logical destination
        uint32_t status     : 1;    //0: Relaxed, 1: Waiting for delivery
        uint32_t polarity   : 1;    //0: Active if high, 1: Active if low
        uint32_t irr        : 1;    //???
        uint32_t trigger    : 1;    //0: Edge, 1: Level
        uint32_t mask       : 1;    //If the IRQ is disabled
        uint32_t reserved   : 32;
        uint32_t reserved2  : 7;
        uint32_t dest       : 8;    //Specific CPU to deliver interrupt to
    } __attribute__((packed));

    struct {
        uint32_t lower;
        uint32_t upper;
    };
} __attribute__((packed)) ioapic_redir_entry_t;

typedef struct ioapic {
    uint8_t *registers;

    uint8_t apic_id;
    uint8_t apic_ver;
    uint8_t count;
    uint8_t base;
} ioapic_t;

extern madt_t *acpi_madt;

extern list_t *acpi_madt_entries;
extern list_t *acpi_topology;

extern uint8_t volatile *apic_registry;

/*  Locates the MADT using the RSDT
 *  Then creates a list of MADT entries in acpi_madt_entries
 * */
void acpi_find_madt(void);

/*  Creates a list of CPU info in acpi_topology
 * */
void acpi_find_topology(void);

/*  Sets up memory regions for use in APIC
 * */
void apic_setup(void);

/*  Enables the APIC for the current processor
 * */
void apic_enable(void);

/*  Acknowlege the interrupt given
 * */
void apic_ack(void);

/*  Reads from the IO/APIC given
 *  io: IO/APIC structure pointer
 *  reg:Register to read
 *  r:  Register value
 * */
uint32_t ioapic_read(ioapic_t *io, uint8_t r);

/*  Writes to the IO/APIC given
 *  io: IO/APIC structure ptr
 *  r:  Register to write
 *  v:  Value to write
 * */
void ioapic_write(ioapic_t *io, uint8_t r, uint32_t v);

#endif
