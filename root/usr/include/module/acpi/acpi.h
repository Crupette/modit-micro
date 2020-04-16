/*  ACPI.H -    Functions to interact with the ACPI
 *
 *  Author: Crupette
 * */

#ifndef MODULE_CPU_ACPI_H
#define MODULE_CPU_ACPI_H 1

#include "kernel/types.h"
#include <stdbool.h>

typedef struct acpisdthdr {
    char sig[4];    //FACP
    uint32_t len;   //Size of the table, including header
    uint8_t rev;    //Revision
    uint8_t chk;    //8 bit checksum field of the whole table
    char oemid[6];  //OEM ID
    char oemtblid[8];   //OEM Table ID
    uint32_t oem_rev;   //OEM Revision
    uint32_t creator_id;    //Creator ID
    uint32_t creator_rev;   //Creator Revision
} acpi_sdt_hdr_t;

typedef struct acpi_gas {
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
} acpi_gas_t;

typedef struct fadt {
    acpi_sdt_hdr_t header;
    uint32_t firmware_ctrl; //32-bit pointer to the FACS.
    uint32_t dsdt;          //32-bit pointer to the DSDT

    uint8_t reserved;   //Only used in ACPI 1.0

    uint8_t preferred_power_managment_profile;
    uint16_t sci_int;   //SCI Interrupt
    uint32_t smi_cmd;   //SMI Command Port
    
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    
    uint8_t s4bios_req;
    
    uint8_t pstate_ctrl;
    
    uint32_t pm1a_evnt_blk;
    uint32_t pm1b_evnt_blk;
    uint32_t pm1a_ctrl_blk;
    uint32_t pm1b_ctrl_blk;
    uint32_t pm2_ctrl_blk;
    uint32_t pm_timer_blk;
    
    uint32_t gpe0_blk;
    uint32_t gpe1_blk;
    
    uint8_t pm1_evnt_len;
    uint8_t pm1_ctrl_len;
    uint8_t pm2_ctrl_len;
    uint8_t pm_timer_len;
    
    uint8_t gpe0_len;
    uint8_t gpe1_len;
    uint8_t gpe1_base;

    uint8_t c_state_ctrl;

    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;

    uint16_t flush_size;
    uint16_t flush_stride;

    uint8_t duty_offset;
    uint8_t duty_width;

    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;

    uint16_t boot_architecture_flags;

    uint8_t reserved2;
    uint32_t flags;

    acpi_gas_t reset_reg;

    uint8_t reset_value;
    uint8_t reserved3[3];

    //Not supporting extended ACPI yet. Keep it 1.0
} fadt_t;

/*  Checks if the table is valid by confirming the table's checksum
 *  header: Header to validate
 *  return: If the header is valid
 * */
bool validate_acpi_table(acpi_sdt_hdr_t *header);

#endif
