#include "module/cpu.h"
#include "module/apic/apic.h"
#include "module/acpi/rsdt.h"
#include "module/interrupt.h"
#include "module/heap.h"

#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/io.h"

madt_t *acpi_madt = 0;
list_t *acpi_madt_entries = 0;

uint8_t volatile *apic_registry = 0;
bool apic_enabled = true;

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

static void disable_pic(){
    //outb(0x20, 0x20);
    //outb(0xA0, 0x20);

    //outb(0x21, 0xFF);
    //outb(0xA1, 0xFF);
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
    //TODO: Replace remap with mmio funcs
    apic_registry = kalloc_a(0x1000, 0x1000);
    virtual_allocator->remappg((void*)0xFEE00000, (void*)apic_registry, 0x3);
}

void _spurious_empty(interrupt_state_t *r){
    (void)r;
}

void _gpf_apic_chk(interrupt_state_t *r){
    log_printf(LOG_WARNING, "Modules do not support this specific APIC setup\n");
    apic_enabled = false;
    r->eip += 2;
}

void apic_enable(void){
    isr_addHandler(13, _gpf_apic_chk);

    uint32_t eax, edx;
    cpu_read_msr(0x1B, &eax, &edx);
    cpu_write_msr(0x1B, eax, edx);

    if(apic_enabled){
        APIC_WRITE(0xE0, 0xFFFFFFFF);
        APIC_WRITE(0xD0, (APIC_READ(0xD0) & 0x00FFFFFF) | 1);
        APIC_WRITE(0x320, 0x10000); //Disable timer
        APIC_WRITE(0x340, 4 << 8);
        APIC_WRITE(0x350, 0x10000);
        APIC_WRITE(0x360, 0x10000);
        APIC_WRITE(0x80, 0);

        eax |= 0x800;

        cpu_write_msr(0x1B,
            eax,
            edx);

        disable_pic(); 

        //Enable spurious interrupt
        APIC_WRITE(0xF0, APIC_READ(0xF0) | 0x100);
    }

    isr_addHandler(13, 0);
}

void apic_ack(void){
    APIC_WRITE(0xB0, 0);
}

void apic_write(uint16_t r, uint32_t val){
    *((uint32_t volatile*)(apic_registry + r)) = val;
}

uint32_t apic_read(uint16_t r){
    return *((uint32_t volatile*)(apic_registry + r));
}


int apic_init(){
    acpi_find_madt();
    
    if(acpi_madt == 0){
        log_printf(LOG_INFO, "ACPI does not have MADT. Reverting to normal PIC\n");
        return 0;
    }
    apic_setup();
    apic_enable();
    return 0;
}

int apic_fini(){
    return 0;
}

module_name(apic);

module_load(apic_init);
module_unload(apic_fini);

module_depends(acpi);
module_depends(dthelper);
module_depends(isr);
module_depends(irq);
module_depends(cpu);
module_depends(heap);
module_depends(paging);
