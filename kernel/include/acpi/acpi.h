#ifndef _KERNEL_ACPI_H
#define _KERNEL_ACPI_H

#include <stdint.h>
#include <bootinfo.h>

typedef struct rsdp {
    char signature[8];
    uint8_t chksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_addr;
    // ACPI 2.0+
    uint32_t len;
    uint64_t xsdt_addr;
    uint8_t ext_chksum;
    uint8_t resv[3];
} __attribute__((packed)) rsdp_t;

typedef struct sdt_hdr {
    char signature[4];
    uint32_t len;
    uint8_t revision;
    uint8_t chksum;
    char oemid[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) sdt_hdr_t;

typedef struct rsdt {
    sdt_hdr_t h;
    uint32_t table_ptr[];
} __attribute__((packed)) rsdt_t;

typedef struct xsdt {
    sdt_hdr_t h;
    uint64_t table_ptr[];
} __attribute__((packed)) xsdt_t;

typedef struct madt {
    sdt_hdr_t h;
    uint32_t lapic_addr;
    uint32_t flags;
} __attribute__((packed)) madt_t;

typedef struct g_addr
{
    uint8_t AddressSpace;
    uint8_t BitWidth;
    uint8_t BitOffset;
    uint8_t AccessSize;
    uint64_t Address;
} __attribute__((packed)) g_addr_t;


typedef struct facp {
    sdt_hdr_t h;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  Reserved;

    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t BootArchitectureFlags;

    uint8_t  Reserved2;
    uint32_t Flags;

    // 12 byte structure; see below for details
    g_addr_t ResetReg;

    uint8_t  ResetValue;
    uint8_t  Reserved3[3];
  
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;

    g_addr_t X_PM1aEventBlock;
    g_addr_t X_PM1bEventBlock;
    g_addr_t X_PM1aControlBlock;
    g_addr_t X_PM1bControlBlock;
    g_addr_t X_PM2ControlBlock;
    g_addr_t X_PMTimerBlock;
    g_addr_t X_GPE0Block;
    g_addr_t X_GPE1Block;
} __attribute__((packed)) facp_t;

sdt_hdr_t *acpi_find_table(const char *signature);
sdt_hdr_t *acpi_find_nth_table(const char *signature, int idx);
void acpi_init(boot_info_t *boot_info);

#endif