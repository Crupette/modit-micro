/*  RSDT.H -    Functions to interact with the RSDT
*
*  Author: Crupette
* */

#include "module/acpi/acpi.h"

#ifndef MODULE_CPU_RSDT_H
#define MODULE_CPU_RSDT_H 1

typedef struct rsdpdesc {
    char sig[8];    //Contains "RSD PTR "
    uint8_t chksum; //Checksum
    char oemid[6];  //OEM ID
    uint8_t rev;    //Revision
    uint32_t rsdt_addr; //Address to the FADT
} __attribute__((packed)) rsdp_descriptor_t;

extern rsdp_descriptor_t *rsdp_descriptor;
extern uintptr_t *rsdt_ptrs;
extern size_t rsdt_entries;

/*  Loads the Root System Description Pointer into rsdp_descriptor
 * */
void load_rsdp(void);

/*  Loads the Root System Description Table into rsdt_start
 * */
void load_rsdt(void);

/*  Finds the requested DT using signature bytes
 *  sig:    Signature of the table
 *  r:      Address of table
 * */
void *rsdt_find_table(char *sig);

#endif
