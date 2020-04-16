#include "module/acpi/acpi.h"
#include "module/acpi/rsdt.h"
#include "module/cpu.h"
#include "module/interrupt.h"

#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/io.h"

bool validate_acpi_table(acpi_sdt_hdr_t *header){
    uint8_t sum = 0;
    for(uint32_t i = 0; i < header->len; i++){
        sum += ((char*)header)[i];
    }
    return sum == 0;
}

int acpi_init(){
    load_rsdp();
    load_rsdt();

    return 0;
}

int acpi_fini(){
    return 0;
}

module_name(acpi);

module_load(acpi_init);
module_unload(acpi_fini);

module_depends(paging);
module_depends(heap);
module_depends(dthelper);
module_depends(idt);
module_depends(isr);
