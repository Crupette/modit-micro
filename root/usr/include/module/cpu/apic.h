/*  APIC.H -    Structures and functions required for the kernel to utilize the APIC
 *
 *  Author: Cupette
 * */
#ifndef MODULE_CPU_APIC_H
#define MODULE_CPU_APIC_H 1

#include "module/cpu/acpi.h"

#include "kernel/types.h"

typedef struct madt {
    acpi_sdt_hdr_t header;
    uint32_t local_apic_addr;
    uint32_t flags;
} madt_t;

typedef struct madt_entry_header {
    uint8_t entry_type;
    uint8_t record_length;
} madt_entry_header_t;

extern madt_t *acpi_madt;

/*  Locates the MADT using the RSDT
 * */
void acpi_find_madt(void);

#endif
