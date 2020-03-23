#include "kernel/initrd.h"
#include "kernel/memory.h"
#include "kernel/print.h"


static uint32_t file_count = 0;
static initrd_file_t *file_list = 0;

extern uintptr_t krnl_next_free_pg;

static uint32_t get_file_size(char *in){
	uint32_t size = 0;
	uint32_t count = 1;

	//File size is in octal, so it needs to be converted to decimal
	for(uint8_t i = 11; i > 0; i--, count *= 8){
		size += ((in[i - 1] - '0') * count);
	}
	return size;
}

static void parse_tar(multiboot_module_t *mod){
	uint32_t addr = mod->mod_start + VIRT_BASE;
	uint32_t tar_files = 0;

	//Number of files is needed first to properly allocate the initrd list.
	for(tar_files = 0; ; tar_files++){
		tar_header_t *header = (tar_header_t*)addr;
		if(header->name[0] == 0) break;
		vga_printf("Found file w name %s\n", header->name);

		uint32_t size = get_file_size((char*)(header->size));
		addr += ((size / 512) + 1) * 512;
		if(size % 512) addr += 512;
	}

	//Initrd needs atleast 1 file
	if(tar_files == 0){
		vga_printf("[ERR]: Either the initrd is empty, or corrupt (no files found)\n");
		return;
	}

	file_list = kvmm_allocpg((void*)(krnl_next_free_pg));
	krnl_next_free_pg += 0x1000;

	addr = mod->mod_start + VIRT_BASE;

	//Converts tar header file to simplified initrd file struct
	for(uint32_t i = 0; i < tar_files; i++){
		tar_header_t *header = (tar_header_t*)addr;

		uint32_t size = get_file_size((char*)(header->size));

		file_list[i].name = header->name;
		file_list[i].header_start = (uint8_t*)header;
		file_list[i].data_start = file_list[i].header_start + 512;
		file_list[i].size = size;

		addr += ((size / 512) + 1) * 512;
		if(size % 512) addr += 512;
	}
	file_count = tar_files;
	vga_printf("[OK]: Loaded initrd with %i files\n", tar_files);
}

initrd_file_t *initrd_get_files(uint32_t *n){
	*n = file_count;
	return file_list;
}

void initrd_init(multiboot_info_t *mbinfo){
	//The 3rd flag tells whether there are modules present
	if((mbinfo->flags) & (1 << 3)){
		multiboot_module_t *mod;
		uint32_t i;
		for(i = 0, mod = (multiboot_module_t*)mbinfo->mods_addr + VIRT_BASE;
			i < mbinfo->mods_count;
			i++, mod++){
			//Module needs to be preserved, so it is not overridden by other allocations
			for(uint32_t j = mod->mod_start; j < mod->mod_end; j += 4096){
				kpmm_resvpg((void*)j);
			}
			//First module is always presumed to be the initrd.
			if(i == 0){	
				parse_tar(mod);
			}
		}
	}else{
		vga_printf("[ERR]: Initrd failed to initialize (no provided initrd)\n");
	}
}
