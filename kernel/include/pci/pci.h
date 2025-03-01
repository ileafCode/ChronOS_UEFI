
#ifndef _KERNEL_PCI_H
#define _KERNEL_PCI_H

#include <stdint.h>
#include <acpi/acpi.h>

typedef struct pci_dev_hdr {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t rev_id;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;
} __attribute__((packed)) pci_dev_hdr_t;

typedef struct pci_hdr0 {
    pci_dev_hdr_t hdr;
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    uint32_t cardbus_CIS_ptr;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t exp_rom_base_addr;
    uint8_t capabilities_ptr;
    uint8_t resv0;
    uint16_t resv1;
    uint32_t resv2;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
} __attribute__((packed)) pci_hdr0_t;

typedef struct dev_cfg {
    uint64_t base_addr;
    uint16_t pci_seg_group;
    uint8_t start_bus;
    uint8_t end_bus;
    uint32_t resv;
} __attribute__((packed)) dev_cfg_t;

typedef struct mcfg {
    sdt_hdr_t h;
    uint64_t resv;
} __attribute__((packed)) mcfg_t;

uint8_t pci_alloc_int_handler();
void pci_init();

#endif