#include "module/user.h"

#include "kernel/elf.h"
#include "kernel/initrd.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/io.h"

uintptr_t modit_elf_load(initrd_file_t *file){
    uint8_t *file_raw = file->data_start;
    size_t file_size = file->size;

    if(file_size == 0) return 0;
    if(file_raw == 0) return 0;

    elf_ehdr_t *header = (elf_ehdr_t*)file_raw;
    if(ELF_CHKHDR(header) == false) return 0;   //Not ELF
    if(header->e_type != ET_EXEC) return 0; //Not executable
    if(header->e_machine != EM_386) return 0;    //Not i386
    if(header->e_version == EV_NONE) return 0;    //Invalid version
    if(header->e_ident[EI_CLASS] != ELFCLASS32) return 0;   //Not x86

    elf_phdr_t *phdrs = (elf_phdr_t*)((uintptr_t)file_raw + header->e_phoff);
    
    for(uint16_t i = 0; i < header->e_phnum; i++){
        elf_phdr_t *phdr = &phdrs[i];

        if(phdr->p_type == PT_LOAD){
            virtual_allocator->allocpgs((void*)phdr->p_vaddr, phdr->p_memsz, 0x7);
            memcpy((void*)phdr->p_vaddr, (file_raw + phdr->p_offset), phdr->p_memsz);
        }
    }

    return header->e_entry;
}
