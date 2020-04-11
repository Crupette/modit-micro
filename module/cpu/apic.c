#include "module/cpu/apic.h"
#include "module/cpu/acpi.h"
#include "module/cpu.h"
#include "module/heap.h"

#include "kernel/memory.h"
#include "kernel/logging.h"

madt_t *acpi_madt = 0;

list_t *acpi_madt_entries = 0;
list_t *acpi_topology = 0;

uint8_t *apic_registry = 0;

void acpi_find_madt(void){
    acpi_madt = (madt_t*)rsdt_find_table("APIC");
    if(acpi_madt == 0){
        log_printf(LOG_INFO, "ACPI has no APIC controller\n");
        return;
    }
    if(validate_acpi_table(&(acpi_madt->header)) == 0){
        log_printf(LOG_WARNING, "MADT has invalid checksum!\n");
        acpi_madt = 0;
        return;
    }

    acpi_madt_entries = new_list();
    madt_entry_header_t *entry = 
        (madt_entry_header_t*)((uint32_t)acpi_madt + sizeof(*acpi_madt));

    while((uintptr_t)entry < (uintptr_t)acpi_madt + acpi_madt->header.len){
        list_push(acpi_madt_entries, entry);
        entry = (madt_entry_header_t*)((uintptr_t)entry + entry->length);
    }
}

void acpi_find_topology(void){
    if(acpi_madt == 0){
        log_printf(LOG_WARNING, "MADT not present! Assuming single core processor\n");
        return;
    }
    acpi_topology = new_list();

    for(list_node_t *node = acpi_madt_entries->head; node; node = node->next){
        if(((madt_entry_header_t*)(node->data))->type == 0){
            list_push(acpi_topology, node->data);
            madt_apic_local_entry_t *cpuent = (madt_apic_local_entry_t*)node->data;
        }
    }
}

static void disable_pic(){
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

bool check_apic(void){
    uint32_t eax, edx;
    cpuid(1, &eax, &edx);
    return edx & CPUID_FEAT_EDX_APIC;
}

void apic_setup(void){
    if(check_apic() == 0){
        log_printf(LOG_WARNING, "CPU doesn't support APIC!\n");
        return;
    }

    //Remap APIC registers to known virtual value
    apic_registry = kalloc_a(0x1000, 0x1000);
    virtual_allocator->remappg((void*)0xFEE00000, apic_registry, 0x3);

    disable_pic();
}

void apic_enable(void){
    //Enable the APIC if not already enabled
    APIC_WRITE(0xF0, APIC_READ(0xF0) | 0x100);
}
