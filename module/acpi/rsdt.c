#include "module/acpi/rsdt.h"
#include "module/heap.h"

#include "kernel/io.h"
#include "kernel/logging.h"
#include "kernel/memory.h"
#include "kernel/string.h"

rsdp_descriptor_t *rsdpd = 0;
uintptr_t *rsdt_ptrs = 0;
size_t rsdt_entries = 0;

void load_rsdp(void){
    uintptr_t rsdp_search_start = 0xE0000 + VIRT_BASE;
    uintptr_t rsdp_search_end = 0xFFFFF + VIRT_BASE;
    char *rsdsig = "RSD PTR ";

    //We are looking for "RSD PTR ", which is always aligned by 16
    for(uintptr_t i = rsdp_search_start; i < rsdp_search_end; i += 16){
        char *search = (char*)i;
        bool found = true;
        for(int j = 0; j < 8; j++){
            if(rsdsig[j] != search[j]) {
                found = false;
                break;
            }
        }
        if(found){
            rsdpd = (rsdp_descriptor_t*)i;
            break;
        }
    }
    if(rsdpd == 0){
        log_printf(LOG_ERROR, "Unable to find RSDP!\n");
        return;
    }

    uint32_t chk = 0;
    for(uint32_t i = 0; i < sizeof(rsdp_descriptor_t); i++){
        chk += ((char*)rsdpd)[i];
    }
    if((chk & 0xFF) != 0){
        log_printf(LOG_ERROR, "RSDP has invalid checksum! (%x)\n", chk);
        rsdpd = 0;
        return;
    }

    char *oemid = kalloc(7);
    memset(oemid, 0, 7);
    for(int i = 0; i < 6; i++) oemid[i] = rsdpd->oemid[i];
    log_printf(LOG_DEBUG, "RSDP OEM %s, Revision %i\n", oemid, rsdpd->rev);
    kfree(oemid);
}

extern uintptr_t krnl_next_free_pg;
void load_rsdt(void){
    if(rsdpd == 0) return;

    uint32_t rsdt_phys = rsdpd->rsdt_addr;
    physical_allocator->resvpg((void*)rsdt_phys);

    void *start = kalloc_a(0x1000, 0x1000);
    //TODO: Replace with mmio function
    virtual_allocator->remappg((void*)rsdt_phys, start, 0x3);
    void *rsdt_start = (void*)((uintptr_t)start + (rsdt_phys % 0x1000));
    
    acpi_sdt_hdr_t *rsdt_hdr = (acpi_sdt_hdr_t*)rsdt_start;
    uint32_t rsdt_len = rsdt_hdr->len;
    rsdt_entries = (rsdt_len - sizeof(*rsdt_hdr)) / sizeof(uintptr_t);
    rsdt_ptrs = (uintptr_t*)((uintptr_t)rsdt_start + sizeof(*rsdt_hdr));

    //Get range for allocation
    uintptr_t min_range = 0xFFFFFFFF;
    uintptr_t max_range = 0;
    for(size_t i = 0; i < rsdt_entries; i++){
        if(rsdt_ptrs[i] > max_range){
            max_range = rsdt_ptrs[i];
        }
        if(rsdt_ptrs[i] < min_range){
            min_range = rsdt_ptrs[i];
        }
    }

    //Pages need to be on page boundary.
    min_range -= (min_range % 0x1000);
    uintptr_t alloc_size = (((max_range / 4096) + 1) * 4096) - min_range;
    void *rsdt_remap = kalloc_a(alloc_size, 0x1000);
    for(size_t i = 0; i < alloc_size; i+= 4096){
        void *remap_phys = (void*)(min_range + i);
        physical_allocator->resvpg(remap_phys);
        //Page trickery to remap pages of the heap to desirable physical memory locations
        //TODO: Replace with mmio function
        virtual_allocator->remappg(remap_phys, (void*)((uintptr_t)rsdt_remap + i), 0x3);
    }

    for(size_t i = 0; i < rsdt_entries; i++){
        //Relocate pointers to virtual memory
        rsdt_ptrs[i] = (uintptr_t)rsdt_remap + (rsdt_ptrs[i] - min_range);
        if(validate_acpi_table((acpi_sdt_hdr_t*)(rsdt_ptrs[i])) == 0){
            log_printf(LOG_WARNING, "Table %i in RSDT table is invalid!\n", i);
        }
    }

}

void *rsdt_find_table(char *sig){
    for(size_t i = 0; i < rsdt_entries; i++){
        if(strncmp(sig, (char*)(rsdt_ptrs[i]), 4) == 0){
            return (void*)(rsdt_ptrs[i]);
        }
    }
    return 0;
}


