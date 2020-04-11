/*  APIC.H -    Structures and functions required for the kernel to utilize the APIC
 *
 *  Author: Cupette
 * */
#ifndef MODULE_CPU_APIC_H
#define MODULE_CPU_APIC_H 1

#include "module/cpu/acpi.h"
#include "module/datatype/list.h"

#include "kernel/types.h"

#define APIC_WRITE(i, value)    ((uint32_t*)(apic_registry + i))[0] = value
#define APIC_READ(i)            *((uint32_t*)(apic_registry + i))

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

extern madt_t *acpi_madt;

extern list_t *acpi_madt_entries;
extern list_t *acpi_topology;

extern uint8_t *apic_registry;

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

#endif
