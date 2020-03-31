#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/print.h"
#include "kernel/io.h"

#include <string.h>

extern multiboot_info_t *mbinfo;

uintptr_t mod_nextfree;
module_info_t modules[32];
uintptr_t modules_list_size;

static elf_shdr_t *k_symtab_ent;
static elf_shdr_t *k_strtab_ent;

typedef int (*module_func)(void);

static void save_ksyms(){
    //Elf stores these variables in mbinfo, but it would be alot of work to just address them as that
    uint32_t shdr_count = mbinfo->u.elf_sec.num;
    uintptr_t stbl_addr = mbinfo->u.elf_sec.addr + VIRT_BASE;   
    struct elf32_shdr *shdr = (struct elf32_shdr*)stbl_addr;

    elf_shdr_t *symtab = 0;
    elf_shdr_t *strtab = 0;

    //Section headers are stored in the section header table, so it must be indexed.
    while((uintptr_t)shdr < stbl_addr + (shdr_count * sizeof(elf_shdr_t))){
        //No specific name is given for sections, so they must be identified by type
        if(shdr->sh_type == SHT_SYMTAB){
            symtab = shdr;
        }
        if(shdr->sh_type == SHT_STRTAB && strtab == 0){ //In many cases, there are multiple strtabs.
                                                        //With the kernel, the first is the one we'er
                                                        //looking for
            strtab = shdr;
        }
        shdr++;

    }

    //Saving for future use
    k_symtab_ent = symtab;
    k_strtab_ent = strtab;
}

//Modules need to link against the kernel, so they need absolute addresses to place symbols
static elf_sym_t *get_ksym(const char *name){
    elf_sym_t *sym = (elf_sym_t*)(k_symtab_ent->sh_addr + VIRT_BASE);
    while((uintptr_t)sym < k_symtab_ent->sh_addr + k_symtab_ent->sh_size + VIRT_BASE){
        if(ELF32_ST_BIND(sym->st_info) == STB_GLOBAL){  //Only global variables should be accessable
                                                        //by kernel modules.
            //Names are stored in the strtab as a list of null-terminated strings back to back
            char *symname = (char*)(k_strtab_ent->sh_addr + VIRT_BASE + sym->st_name);
            if(strcmp(symname, name) == 0) return sym;
        }
        sym++;
    }
    return 0;
}

void kmod_init(){
    //GRUB should load the symtab and strtab with the kernel. if not, modules are not possible
    if((mbinfo->flags & MULTIBOOT_INFO_ELF_SHDR) == 0){
        vga_printf("[ERR]: Kernel not loaded with elf shdr\n");
        return;
    }
    save_ksyms(mbinfo);

    //This address is where new modules will be placed. After this, the number will be incremented by
    //the space taken up by said module
    mod_nextfree = MODULES_START;
    modules_list_size = 0;
    memset(modules, 0, sizeof(modules));

    uint32_t filecount = 0;
    initrd_file_t *files = initrd_get_files(&filecount);

    for(uint32_t i = 0; i < filecount; i++){
        initrd_file_t *file = &(files[i]);
        size_t namelen = strlen(file->name);
        //We should only load kernel modules into kernel-land. 
        //If a usermode program gets by, it will be blocked because it's not relocatable
        if(file->name[namelen - 3] != '.' ||
            file->name[namelen - 2] != 'k' ||
            file->name[namelen - 1] != 'o'){
            continue;
        }

        kmod_load(file);
    }
}

static void load_dependency(module_info_t *module, char *dep){
    for(uint8_t i = 0; i < modules_list_size; i++){
        module_info_t *omod = &modules[i];
        if(strcmp(omod->name, dep) == 0){
            modules->deplist[module->deps++] = i;
            return;
        }
    }
    char namebuf[128];
    memset(namebuf, 0, 128);
    strcpy(namebuf, dep);
    strcpy(namebuf + strlen(dep), ".ko");

    uint32_t filecount = 0;
    initrd_file_t *files = initrd_get_files(&filecount);
    for(initrd_file_t *file = files; file < &files[filecount]; file++){
        if(strcmp(file->name, namebuf) == 0){
            int loaded = kmod_load(file);
            if(loaded == -1){
                vga_printf("Module dependency [%s] failed to load\n", dep);
                return;
            }
            modules->deplist[module->deps++] = loaded;
            return;
        }
    }
    vga_printf("Failed to load module dependency [%s]\n", dep);
}

static elf_sym_t *mod_findsym(module_info_t *mod, const char *name){
    elf_sym_t *ksym = get_ksym(name);
    if(ksym == NULL){
        //We need to look in the dependencies.
        for(int i = 0; i < mod->deps; i++){
            module_info_t *depmod = &modules[i];
            
            elf_sym_t *symtab = depmod->symtab;
            char *symstrtab = depmod->strtab;

            for(elf_sym_t *depsym = symtab; 
                    (uintptr_t)depsym < depmod->symtab_hdr->sh_size;
                    depsym++){
                char *depsymname = &symstrtab[depsym->st_name];
                if(strcmp(depsymname, name) == 0) return depsym;
            }
        }
    }
    return ksym;
}

int kmod_load(initrd_file_t *file){
    for(uint8_t i = 0; i < modules_list_size; i++){
        if(modules[i].file == file) return i;
    }
    
    uint8_t *file_raw = file->data_start;
    uint32_t file_size = file->size;
    
    //File needs to have data
    if(file_size == 0) return -1;
    if(file_raw == 0) return -1;

    //Must be an ELF Relocatable
    elf_ehdr_t *header = (elf_ehdr_t*)file_raw;
    if(ELF_CHKHDR(header) == false) return -1;
    if(header->e_type != ET_REL) return -1;

    elf_shdr_t *shdrs = (elf_shdr_t*)((uintptr_t)file_raw + header->e_shoff);
    uint8_t *shstrtab = (uint8_t*)((uintptr_t)file_raw + shdrs[header->e_shstrndx].sh_offset);

    elf_shdr_t *symtab = 0;
    elf_shdr_t *strtab = 0;

    uintptr_t newalloc = mod_nextfree;
    //Locate symtab and symstrtab, load all loadable sections into memory
    for(uint16_t i = 0; i < header->e_shnum; i++){
        elf_shdr_t *shdr = &shdrs[i];
        char *name = (char*)((uintptr_t)shdr->sh_name + (uintptr_t)shstrtab);
        if(strcmp(name, ".symtab") == 0)
            symtab = shdr;
        if(strcmp(name, ".strtab") == 0)
            strtab = shdr;

        if(shdr->sh_flags & SHF_ALLOC ||
           shdr->sh_type == SHT_SYMTAB ||
           shdr->sh_type == SHT_STRTAB){
            uint32_t align = shdr->sh_addralign;
            while(newalloc % align != 0) newalloc++;
            for(uintptr_t j = newalloc; j < newalloc + shdr->sh_size + 4096; j += 4096){
                virtual_allocator->allocpg((void*)j, false, 
                        shdr->sh_flags & SHF_WRITE);
            }
            shdr->sh_addr = newalloc;

            //BSS needs to be dynamically allocated
            if(shdr->sh_type == SHT_NOBITS){
                memset((void*)shdr->sh_addr, 0x0, shdr->sh_size);
            }else{
                memcpy((void*)shdr->sh_addr,
                        (void*)((uintptr_t)file_raw + shdr->sh_offset),
                        shdr->sh_size);
            }

            newalloc += (shdr->sh_size + 1);
        }
    }
    mod_nextfree = newalloc;
    if(symtab == 0 || strtab == 0){
        vga_printf("[ERR]: %s symtab %p, strtab %p\n", file->name, symtab, strtab);
        return -1;
    }

    elf_sym_t *symtab_actual = (elf_sym_t*)symtab->sh_addr;
    char *strtab_actual = (char*)strtab->sh_addr;

    unsigned int id = modules_list_size;
    module_info_t *module = &modules[modules_list_size++];
    module->file = file;
    module->sheaders = shdrs;
    module->symtab = symtab_actual;
    module->strtab = strtab_actual;

    module->symtab_hdr = symtab;
    module->strtab_hdr = strtab;
    
    //Parse dependencies
    for(elf_sym_t *sym = symtab_actual; 
            (uintptr_t)sym < symtab->sh_addr + symtab->sh_size; 
            sym++){
        if(sym->st_shndx > 0xFFF) continue;
        char *symname = &strtab_actual[sym->st_name];
        char *hdrname = (char*)(
                shdrs[(uintptr_t)sym->st_shndx].sh_name + (uintptr_t)shstrtab);
        //Dependencies MUST be placed in section .moddeps and prepended by _module_depends_
        if(strcmp(hdrname, ".moddeps") == 0){
            if(strstr(symname, "_module_depends_") != NULL){
                load_dependency(module, symname + strlen("_module_depends_"));
            }
        }
    }

    //Process relocatables
    for(uint16_t i = 0; i < header->e_shnum; i++){
        elf_shdr_t *shdr = &shdrs[i];
        if(shdr->sh_type == SHT_REL){
            //Get the relocatable objects from header data
            elf_rel_t *rels = (elf_rel_t*)(shdr->sh_offset + (uintptr_t)file_raw);
            elf_shdr_t *relsec = &shdrs[shdr->sh_info];
            //Do not relocate symbols in an un-loaded section
            if(relsec->sh_flags == 0) continue;
            for(uint32_t j = 0; j < shdr->sh_size / shdr->sh_entsize; j++){
                uint16_t relsymi = ELF32_R_SYM(rels[j].r_info);
                elf_sym_t *relsym = &symtab_actual[relsymi];
                char *relsymname = (char*)((uintptr_t)strtab_actual + relsym->st_name);
                uint32_t symval = relsym->st_value;
                uint32_t *relocat = (uint32_t*)((uintptr_t)relsec->sh_addr + rels[j].r_offset);

                //Symbol is a section. No special rules are needed
                if(ELF32_ST_TYPE(relsym->st_info) == STT_SECTION){
                    elf_shdr_t *relsecsec = &shdrs[relsym->st_shndx];
                    symval = relsecsec->sh_addr;
                }else
                //Symbol is not in file
                if(relsym->st_shndx == SHN_UNDEF){
                    elf_sym_t *newsym = mod_findsym(module, relsymname);
                    if(newsym == 0){
                        vga_printf("Could not find symbol [%s] in the kernel or the module's dependencies!\n",
                                relsymname);
                        return -1;
                    }
                    symval = newsym->st_value;
                }else{
                    if(relsym->st_shndx == SHN_ABS){
                        symval = relsym->st_value;
                    }else
                    if(relsym->st_shndx == SHN_COMMON){
                        vga_printf("Sym [%s] requested illegal relocation\n",
                                relsymname);
                        return -1;
                    }else{
                        elf_shdr_t *symtgt = &shdrs[relsym->st_shndx];
                        symval = symtgt->sh_addr + relsym->st_value;
                    }
                }
                switch(ELF32_R_TYPE(rels[j].r_info)){
                    case R_386_NONE:
                        vga_printf("Relocation in %s (%i) is invalid!\n",
                            (char*)((uintptr_t)shdr->sh_name + 
                                (uintptr_t)shstrtab),
                            i);
                    break;
                    case R_386_32:
                        *relocat = symval + (uintptr_t)(*relocat);
                    break;
                    case R_386_PC32:
                        *relocat = (symval + (uintptr_t)(*relocat)) - (relsec->sh_addr + rels[j].r_offset);
                    break;
                }
            }
        }
    }

    //Look for _module_name, _module_load, _module_unload
    for(uint32_t i = 0; i < symtab->sh_size / symtab->sh_entsize; i++){
        elf_sym_t *sym = &symtab_actual[i];
        char *symname = (char*)((uintptr_t)strtab_actual + sym->st_name);
        
        if(sym->st_shndx > 0xFFF) continue;
        sym->st_value = shdrs[sym->st_shndx].sh_addr + sym->st_value;

        //We need to treat this differently, and I want to not copy code
        if(strcmp(symname, "_module_name") == 0 ||
           strcmp(symname, "_module_load") == 0 ||
           strcmp(symname, "_module_unload") == 0){
            void *value = (void*)sym->st_value;

            if(strcmp(symname, "_module_name") == 0){
                char *name = (char*)(*((uintptr_t*)value));
                module->name = name;
            }

            if(strcmp(symname, "_module_load") == 0){
                module_func ptr = (module_func)(*((uintptr_t*)value));
                module->enter = ptr;
            }

            if(strcmp(symname, "_module_unload") == 0){
                module_func ptr = (module_func)(*((uintptr_t*)value));
                module->exit = ptr;
            }
        }
    }

    if(module->name == 0){
        vga_printf("Module in file [%s] is missing a name definition!\n", file->name);
        return -1;
    }
    if(module->enter == 0){
        vga_printf("Module [%s] has no entry point!\n", module->name);
        vga_printf("  * Define this using the macro [module_load]\n");
        vga_printf("  * with a pointer to the entry function\n");
        return -1;
    }
    if(module->exit == 0){
        vga_printf("Module [%s] has no exit point!\n", module->name);
        vga_printf("  * Define this using the macro [module_unload]\n");
        vga_printf("  * with a pointer to the exit function\n");
        return -1;
    }

    int ret = module->enter();
    if(ret != 0){
        vga_printf("Module %s returned with code %i\n", module->name, ret);
    }
    return id;
}

