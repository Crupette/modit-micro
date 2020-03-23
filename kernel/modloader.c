#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/print.h"

#include <string.h>

uintptr_t mod_nextfree;

static elf_shdr_t *k_symtab_ent;
static elf_shdr_t *k_strtab_ent;

static void save_ksyms(multiboot_info_t *mbinfo){
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
		if(shdr->sh_type == SHT_STRTAB && strtab == 0){	//In many cases, there are multiple strtabs.
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
		if(ELF32_ST_BIND(sym->st_info) == STB_GLOBAL){	//Only global variables should be accessable
								//by kernel modules.
			//Names are stored in the strtab as a list of null-terminated strings back to back
			char *symname = (char*)(k_strtab_ent->sh_addr + VIRT_BASE + sym->st_name);
			if(strcmp(symname, name) == 0) return sym;
		}
		sym++;
	}
	return 0;
}

void kmod_init(multiboot_info_t *mbinfo){
	//GRUB should load the symtab and strtab with the kernel. if not, modules are not possible
	if((mbinfo->flags & MULTIBOOT_INFO_ELF_SHDR) == 0){
		vga_printf("[ERR]: Kernel not loaded with elf shdr\n");
		return;
	}
	save_ksyms(mbinfo);

	//This address is where new modules will be placed. After this, the number will be incremented by
	//the space taken up by said module
	mod_nextfree = 0xD8000000;

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

void kmod_load(initrd_file_t *file){
	uint8_t *file_raw = file->data_start;
	uint32_t file_size = file->size;
	
	if(file_size == 0) return;
	if(file_raw == 0) return;

	elf_ehdr_t *header = (elf_ehdr_t*)file_raw;
	if(ELF_CHKHDR(header) == false) return;
	if(header->e_type != ET_REL) return;

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
			uint32_t align = shdr->sh_addralign - 1;
			newalloc = ((newalloc >> align) + 1) << align;
			vga_printf("Loading section [%s] into addr %p -> %p (%x)\n", name, newalloc, newalloc + shdr->sh_size, shdr->sh_addralign);
			for(int i = newalloc; i < newalloc + shdr->sh_size; i += 4096){
				kvmm_allocpg((void*)i);
			}
			shdr->sh_addr = newalloc;

			if(shdr->sh_type == SHT_NOBITS){
				memset((void*)shdr->sh_addr, 0x0, shdr->sh_size);
			}else{
				memcpy((void*)shdr->sh_addr,
			     		(void*)((uintptr_t)file_raw + shdr->sh_offset),
			     		shdr->sh_size);
			}

			newalloc += (shdr->sh_size - 1);
		}
	}
	mod_nextfree = newalloc;

	if(symtab == 0 || strtab == 0){
		vga_printf("[ERR]: Module symtab %p, strtab %p\n", symtab, strtab);
	}

	elf_sym_t *symtab_actual = (elf_sym_t*)((uintptr_t)file_raw + symtab->sh_offset);
	char *strtab_actual = (char*)((uintptr_t)file_raw + strtab->sh_offset);

	//Process relocatables
	for(uint16_t i = 0; i < header->e_shnum; i++){
		elf_shdr_t *shdr = &shdrs[i];
		if(shdr->sh_type == SHT_REL){
			//Get the relocatable objects from header data
			elf_rel_t *rels = (elf_rel_t*)(shdr->sh_offset + (uintptr_t)file_raw);
			elf_shdr_t *relsec = &shdrs[shdr->sh_info];
			//Do not relocate symbols in an un-loaded section
			if(relsec->sh_flags == 0) continue;
			vga_printf("Section %s is relocating for %s\n",
					(char*)((uintptr_t)shdr->sh_name + (uintptr_t)shstrtab),
					(char*)((uintptr_t)relsec->sh_name + (uintptr_t)shstrtab));
			for(int j = 0; j < shdr->sh_size / shdr->sh_entsize; j++){
				uint16_t relsymi = ELF32_R_SYM(rels[j].r_info);
				elf_sym_t *relsym = &symtab_actual[relsymi];
				char *relsymname = (char*)((uintptr_t)strtab_actual + relsym->st_name);
				uint32_t symval = relsym->st_value;

				//Symbol is not in file
				if(relsym->st_shndx == SHN_UNDEF){
					elf_sym_t *ksym = get_ksym(relsymname);
					if(ksym == 0){
						vga_printf("Could not find symbol %s in kernel\n", relsymname);
						vga_printf("Load failed!\n");
						return;
					}
					symval = ksym->st_value;
				}else{
					if(relsym->st_shndx == SHN_ABS){
						symval = relsym->st_value;
					}else{
						elf_shdr_t *symtgt = &shdrs[relsym->st_shndx];
						symval = symtgt->sh_addr + relsym->st_value;
					}
				}

				uint32_t *relocat = (uint32_t*)((uintptr_t)relsec->sh_addr + rels[j].r_offset);

				switch(ELF32_R_TYPE(rels[j].r_info)){
					case R_386_NONE:
						vga_printf("Relocation in %s (%i) is invalid!\n",
							(char*)((uintptr_t)shdr->sh_name + (uintptr_t)shstrtab),
							i);
					break;
					case R_386_32:
						vga_printf("R386_32 Relocating sym %s [%p] (%p) to ", 
								relsymname, *relocat, relocat);
						*relocat = symval + *relocat;
					break;
					case R_386_PC32:
						vga_printf("R386_PC32 Relocating sym %s [%p] (%p) to ", 
								relsymname, *relocat, relocat);
						*relocat = (symval + *relocat) - (relsec->sh_addr + rels[j].r_offset);
					break;
				}
				vga_printf("%p\n", *relocat);

			}
		}
	}

	//Look for _module_name, _module_load, _module_unload
	for(int i = 0; i < symtab->sh_size / symtab->sh_entsize; i++){
		elf_sym_t *sym = &symtab_actual[i];
		char *symname = (char*)((uintptr_t)strtab_actual + sym->st_name);

		//We need to treat this differently, and I want to not copy code
		if(strcmp(symname, "_module_name") == 0 ||
		   strcmp(symname, "_module_load") == 0 ||
		   strcmp(symname, "_module_unload") == 0){
			elf_shdr_t *sym_target = &shdrs[sym->st_shndx];
			void *value = (void*)(sym_target->sh_addr + sym->st_value);

			if(strcmp(symname, "_module_name") == 0){
				char *name = (char*)(*((uintptr_t*)value));
				vga_printf("Found module name: [%s] (%p)\n", name, name);
			}

			if(strcmp(symname, "_module_load") == 0){
				void *ptr = (void*)(*((uintptr_t*)value));
				vga_printf("Found entry: %p\n", ptr);
			}

			if(strcmp(symname, "_module_unload") == 0){
				void *ptr = (void*)(*((uintptr_t*)value));
				vga_printf("Found exit: %p\n", ptr);
			}
		}
	}
}

