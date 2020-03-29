/*	ELF.H - Provides defines and data structures for programs wishing to
 *		read ELF files and do things with it's data structures.
 *		Based off www.sco.com/developers/gabi/latest/contents.html
 *
 *	Author:	Crupette
 * */

//TODO: 64 bit functions
#ifndef STD_ELF_H
#define STD_ELF_H 1

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define EI_NIDENT	16	//Size of e_ident[]

struct elf32_ehdr {
	unsigned char e_ident[EI_NIDENT];	//Initial bytes marking the presense of ELF data
	uint16_t e_type;	//Object file type
	uint16_t e_machine;	//Required architecture for specific file (There are tons of values)
	uint32_t e_version;	//Object file version
	uintptr_t e_entry;	//Virtual address of beginning of execution
	uintptr_t e_phoff;	//Program headers table file offset
	uintptr_t e_shoff;	//Section headers table file offset
	uint32_t e_flags;	//Processor specific flags
	uint16_t e_ehsize;	//ELF header size
	uint16_t e_phentsize;	//Size of one entry in the program header table
	uint16_t e_phnum;	//Number of entries in program header table
	uint16_t e_shentsize;	//Size of one entry in the section header table
	uint16_t e_shnum;	//Number of entries in section header table
	uint16_t e_shstrndx;	//Section header table index for the section name string table
};

#define ET_NONE		0	//No file type
#define ET_REL		1	//Relocatable
#define ET_EXEC		2	//Executable
#define ET_DYN		3	//Shared object (library)
#define ET_CORE		4	//Core file
#define ET_LOOS		0xFE00	//OS Specific
#define ET_HIOS		0xFEFF	//OS Specific
#define ET_LOPROC	0xFF00	//Processor specific
#define ET_HIPROC	0xFFFF	//Processor specific

#define EI_MAG0		0	//Magic number index 0
#define EI_MAG1		1	//Magic number index 1
#define EI_MAG2		2	//Magic number index 2
#define EI_MAG3		3	//Magic number index 3
#define EI_CLASS	4	//File class
#define EI_DATA		5	//Data encoding scheme
#define EI_VERSION	6	//File version
#define EI_OSABI	7	//ABI Identification / Operating system
#define EI_ABIVER	8	//ABI Version
#define EI_PAD		9	//Padding

#define EV_NONE		0	//Invalid version
#define EV_CURRENT	1	//Current version

#define ELFMAG0		0x7f
#define ELFMAG1		'E'
#define ELFMAG2		'L'
#define ELFMAG3		'F'

#define ELFCLASSNONE	0	//Invalid class
#define ELFCLASS32	1	//32-bit object
#define ELFCLASS64	2	//64-bit object

#define ELFDATANONE	0	//Invalid data encoding
#define ELFDATA2LSB	1	//Least significant byte
#define ELFDATA2MSB	2	//Most significant byte

#define ELF_CHKHDR(hdr) (hdr->e_ident[EI_MAG0] == ELFMAG0 && \
			 hdr->e_ident[EI_MAG1] == ELFMAG1 && \
			 hdr->e_ident[EI_MAG2] == ELFMAG2 && \
			 hdr->e_ident[EI_MAG3] == ELFMAG3)

struct elf32_shdr {
	uint32_t	sh_name;	//Name of the section as index into the shstrtab
	uint32_t	sh_type;	//Sector type as specified by SHT_X defines
	uint32_t	sh_flags;	//1-byte flag values as specified by SHF_X defines
	uint32_t	sh_addr;	//Address of placement in ram for section data. 0 if non-physical
	uint32_t	sh_offset;	//Index in file to find data
	uint32_t	sh_size;	//Size in bytes of data
	uint32_t	sh_link;	//Section header table index link, as defined by SHT_X defines
	uint32_t	sh_info;	//Holds sh_type dependent information, if SHF_INFO_LINK set, represents section header table index
	uint32_t	sh_addralign;	//Defines alignment of memory placed in physical memory.
	uint32_t	sh_entsize;	//Size of bytes of each entry defined in data region, such as symtab. Otherwise 0
};

#define SHT_NULL		0	//Inactive header
#define SHT_PROGBITS		1	//Information relevant for running a program
#define SHT_SYMTAB		2	//Symbol table, used for program linking
#define SHT_STRTAB		3	//String table, symbols are given names for future linking / reference
#define SHT_RELA		4	//Relocation entries with explicit addends
#define SHT_HASH		5	//Symbol hash table. One per obj file
#define SHT_DYNAMIC		6	//Information for dynamic linking. One per obj file
#define SHT_NOTE		7	//Marks the file in some way, used to check for compatability, etc
#define SHT_NOBITS		8	//Section with no information, otherwise conforming to PROGBITS
#define SHT_REL			9	//Relocation entries without explicit addends
#define SHT_SHLIB		10	//Reserved, has unspecified semantics
#define SHT_DYNSYM		11	//Symbol table, not special to SYMTAB
#define SHT_INIT_ARRAY		12	//Array of pointers to initialization functions
#define SHT_FINI_ARRAY		13	//Array of pointers to termination functions
#define SHT_PREINIT_ARRAY	14	//Array of pointers to be invoked before initialization functions
#define SHT_GROUP		15	//Section group. These sections are related and must be treated specially
#define SHT_SYMTAB_SHNDX	16	//Needed if any symbol contains escape value SHN_XINDEX

#define SHF_WRITE		0x1	//Should be writeable during process execution
#define SHF_ALLOC		0x2	//Occupies memory during process execution
#define SHF_EXECINSTR		0x4	//Contains executable machine instructions
#define SHF_MERGE		0x10	//May be merged to eliminate duplication, unless SHF_STRINGS is set
#define SHF_STRINGS		0x20	//Consists of null-terminated character strings
#define SHF_INFO_LINK		0x40	//sh_info field holds section header table index
#define SHF_LINK_ORDER		0x80	//Adds special ordering requirements for link editors.
#define SHF_OS_NONCONFORMING	0x100	//Requires specific OS-specific processing to avoid incorrect behavior
#define SHF_GROUP		0x200	//Is a member of a section group. Must be referenced by a section of SHT_GROUP
#define SHF_TLS			0x400	//Holds thread-local storage, each execution has its own instance
#define SHF_COMPRESSED		0x800	//Contains compressed data. Can not be used with SHF_ALLOC / SHT_NOBITS
#define SHF_MASKOS		0x0FF00000
#define SHF_MASKPROC		0xF0000000

#define SHN_UNDEF		0
#define SHN_ABS			0xFFF1
#define SHN_COMMON		0xFFF2

struct elf32_chdr {
	uint32_t ch_type;	//Specifies compression algorithm.
	uint32_t ch_size;	//Size in bytes of un-compressed data
	uint32_t ch_addralign;	//Alignment of un-compressed data
};

#define ELFCOMPRESS_ZLIB	1	//Section data is compressed with ZLIB

struct elf32_sym {
	uint32_t st_name;	//Index into symstrtab. If zero, symbol has no name
	uint32_t st_value;	//Value of symbol
	uint32_t st_size;	//Size of symbol. 0 if no / unknown size
	uint8_t st_info;	//Symbol type and binding attributes
	uint8_t st_other;	//Specifies visibility
	uint16_t st_shndx;	//Section header table index
};

#define ELF32_ST_BIND(i)	((i) >> 4)
#define ELF32_ST_TYPE(i)	((i) & 0xF)
#define ELF32_ST_INFO(b, t)	(((b) << 4) + ((t) & 0xF))
#define ELF32_ST_VISIBILITY(o)	((o)&0x3)

//Symbol Bindings
#define STB_LOCAL	0	//Local symbols not visible outside of defining file
#define STB_GLOBAL	1	//Visible to all files being combined. 
#define STB_WEAK	2	//Same as global, but can be overridden by GLOBAL variables

//Symbol Types
#define STT_NONE	0	//Not specified
#define STT_OBJECT	1	//Data object (eg, var, array)
#define STT_FUNC	2	//Function / executable code
#define STT_SECTION	3	//Section. for relocation purposes.
#define STT_FILE	4	//Symbols name gives name of the source file associated with object file
#define STT_COMMON	5	//Uninitialized common block
#define STT_TLS		6	//Thread-local storage entity

//Symbol visibility
#define STV_DEFAULT	0	//Visibility is defined by binding type
#define STV_INTERNAL	1	//Complicated
#define STV_HIDDEN	2	//Hidden if name not visible to other components
#define STV_PROTECTED	3	//Visible in other components, but not preemptable

struct elf32_rel {
	uint32_t r_offset;	//Location at which to apply the relocation action.
					//For reloc file, value is offset from beginning of the section into the storage unit affected
					//For executable / shobj value is virtual address of the storage unit.
	uint32_t r_info;	//Gives symtab index and type of relocation to apply.
};

struct elf32_rela {
	uint32_t r_offset;
	uint32_t r_info;
	uint32_t r_addend;	//Specifies a constant addend used to compute value stored in reloc field
};

#define ELF32_R_SYM(i)		((i) >> 8)
#define ELF32_R_TYPE(i)		((unsigned char)i)
#define ELF32_R_INFO(s, t)	(((s) << 8) + (unsigned char)(t))

#define R_386_NONE	0	//No relocation
#define R_386_32	1	//Symbol + Offset
#define R_386_PC32	2	//Symbol + Offset - Section Offset

struct elf32_phdr {
	uint32_t p_type;	//What kind of segment this describes / how to interpret it's data
	uint32_t p_offset;	//Offset from beginning of file to find the data described by this segment
	uint32_t p_vaddr;	//Virtual address where the segment resides in memory
	uint32_t p_paddr;	//Segment's physical address (where applicable)
	uint32_t p_filesz;	//Number of bytes in the file image of the segment. may be 0
	uint32_t p_memsz;	//Number of bytes in the memory image of the segment. may be 0
	uint32_t p_flags;	//flags relevant to the segment
	uint32_t p_align;	//Loaded address must be (addr % p_align) == 0
};

#define PT_NULL		0	//Element is unused
#define PT_LOAD		1	//Loadable segment, described by p_filesz and p_memsz, mapped to p_vaddr
#define PT_DYNAMIC	2	//Dynamic linking information
#define PT_INTERP	3	//Location and size of null-terminated path name to act as interpreter
#define PT_NOTE		4	//Location and size of auxiliary information
#define PT_SHLIB	5	//Reserved, unspecified semantics
#define PT_PHDR		6	//Location and size of program header table
#define PT_TLS		7	//Thread-local storage template.

#define PF_X		0x1	//Execute
#define PF_W		0x2	//Write
#define PF_R		0x4	//Read

#ifdef BITS32
typedef struct elf32_ehdr elf_ehdr_t;
typedef struct elf32_shdr elf_shdr_t;
typedef struct elf32_phdr elf_phdr_t;
typedef struct elf32_chdr elf_chdr_t;
typedef struct elf32_sym elf_sym_t;
typedef struct elf32_rel elf_rel_t;
typedef struct elf32_rela elf_rela_t;
#else

#endif

#endif 
