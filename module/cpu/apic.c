#include "module/cpu/apic.h"
#include "module/cpu/acpi.h"
#include "module/heap.h"

#include "kernel/memory.h"
#include "kernel/logging.h"

madt_t *acpi_madt = 0;

void acpi_find_madt(void){
    acpi_madt = (madt_t*)rsdt_find_table("APIC");
    if(acpi_madt == 0){
        log_printf(LOG_INFO, "ACPI has no APIC controller\n");
        return;
    }
    if(validate_acpi_table(&(acpi_madt->header)) == 0){
        log_printf(LOG_WARNING, "MADT has invalid checksum!\n");
    }
}
