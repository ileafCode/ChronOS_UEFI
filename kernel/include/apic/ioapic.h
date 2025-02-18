
#ifndef _KERNEL_IOAPIC_H
#define _KERNEL_IOAPIC_H

#include <stdint.h>
#include <acpi/acpi.h>

#define IOAPIC_AR 0x00
#define IOAPIC_DR 0x10

#define IOAPIC_ENTRY_MASK (1 << 16)

typedef struct madt_vlr {
	uint8_t entry_type;
	uint8_t rec_length;
} __attribute__((packed)) madt_vlr_t;

typedef struct ioapic_entry {
    madt_vlr_t vlr;
    uint8_t ioapic_id;
    uint8_t resv;
    uint32_t ioapic_addr;
    uint32_t gsib; // Global System Interrupt Base
} __attribute__((packed)) ioapic_entry_t;

typedef struct apic_int_override {
    madt_vlr_t header;
    uint8_t bus;
    uint8_t source;
    uint32_t interrupt;
    uint16_t flags;
} __attribute__((packed)) apic_int_override_t;

int  ioapic_remap_irq(int irq);
void ioapic_set_entry(uint8_t index, uint64_t data);
void ioapic_init();

#endif