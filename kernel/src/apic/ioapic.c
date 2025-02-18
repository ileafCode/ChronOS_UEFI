#include <apic/ioapic.h>
#include <mm/vmm/paging.h>
#include <printk/printk.h>

madt_t *madt;
volatile uint32_t* ioapic_address;

static inline void write_ioapic_register(uint8_t offset, uint32_t val) {
    *(volatile uint32_t *)(ioapic_address + (IOAPIC_AR / sizeof(uint32_t))) = offset;
    __asm__ volatile ("mfence" ::: "memory");  // Ensure ordering
    *(volatile uint32_t *)(ioapic_address + (IOAPIC_DR / sizeof(uint32_t))) = val;
}

static inline uint32_t read_ioapic_register(uint8_t offset) {
    *(volatile uint32_t *)(ioapic_address + (IOAPIC_AR / sizeof(uint32_t))) = offset;
    __asm__ volatile ("mfence" ::: "memory");  // Ensure previous write completes
    return *(volatile uint32_t *)(ioapic_address + (IOAPIC_DR / sizeof(uint32_t)));
}

int ioapic_remap_irq(int irq) {
    if (!madt) return irq;  // Ensure MADT exists

    uint8_t *p = (uint8_t *)(madt + 1);
    uint8_t *end = (uint8_t *)madt + madt->h.len;

    while (p < end) {
        madt_vlr_t *header = (madt_vlr_t *)p;

        // Prevent infinite loops due to corrupted tables
        if (header->rec_length < sizeof(madt_vlr_t)) break;

        if (header->entry_type == 2) {  // Type 2 = Interrupt Source Override
            apic_int_override_t *s = (apic_int_override_t *)p;
            //printk("Int: %d; Src: %d\n", s->interrupt, s->source);
            if (s->source == irq)
                return s->interrupt;
        }

        p += header->rec_length;
    }

    return irq;
}

void ioapic_set_entry(uint8_t index, uint64_t data) {
    write_ioapic_register(0x10 + index * 2, (uint32_t)data);
    write_ioapic_register(0x10 + index * 2 + 1, (uint32_t)(data >> 32));
}

void ioapic_init() {
    madt = (madt_t *)acpi_find_table("APIC");
    if (!madt) {
        printk("No IOAPIC. Halting.\n");
        asm volatile ("cli;hlt");
    }

    uint8_t *entry = (uint8_t *)madt + sizeof(madt_t);
    uint8_t *madt_end = (uint8_t *)madt + madt->h.len;  // End of MADT
    
    ioapic_entry_t *ioapic_entry = 0;

    while (entry < madt_end) {
        madt_vlr_t *vlr = (madt_vlr_t *)entry;
        switch (vlr->entry_type) {
            case 1: // IO APIC
                printk("IO APIC found\n");
                ioapic_entry = (ioapic_entry_t *)vlr;
                break;
        }

        entry += vlr->rec_length;
    }

    ioapic_address = (uint32_t*)ioapic_entry->ioapic_addr;
    paging_map(
        (void *)((uint64_t)ioapic_address & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)ioapic_address & 0xFFFFFFFFFFFFF000),
        (enum ptflag[]){ CacheDisabled, End }
    );

    uint32_t ioapic_reg1 = read_ioapic_register(0x01);
    uint32_t max_entries = ((ioapic_reg1 >> 16) & 0xff) + 1;

    for (uint32_t i = 0; i < max_entries; ++i)
        ioapic_set_entry(i, IOAPIC_ENTRY_MASK);
    
    printk("IOAPIC initialized\n");
}