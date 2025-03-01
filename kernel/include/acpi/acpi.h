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

typedef struct bgrt {
    sdt_hdr_t h;
    uint16_t ver_id;
    uint8_t status;
    uint8_t image_type;
    uint64_t image_addr;
    uint32_t image_x_off;
    uint32_t image_y_off;
} __attribute__((packed)) bgrt_t;

sdt_hdr_t *acpi_find_table(const char *signature);
sdt_hdr_t *acpi_find_nth_table(const char *signature, int idx);
void acpi_init(boot_info_t *boot_info);

#endif