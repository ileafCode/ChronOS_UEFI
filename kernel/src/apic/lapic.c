#include <apic/lapic.h>
#include <apic/lapic.h>
#include <printk/printk.h>
#include <mm/vmm/paging.h>
#include <io/io.h>

volatile uint32_t *lapic_address = 0;

void write_lapic_register(const uint32_t offset, const uint32_t val) {
    *((volatile uint32_t*)((uintptr_t)lapic_address + offset)) = val;
}

uint32_t read_lapic_register(const uint32_t offset) {
    return *((volatile uint32_t*)((uintptr_t)lapic_address + offset));
}
uint32_t get_lapic_addr() {
    return (uint32_t)((uint64_t)lapic_address);
}

void lapic_init() {
    madt_t *madt = (madt_t *)acpi_find_table("APIC");
    if (!madt) {
        printk("No LAPIC. Halting.\n");
        asm volatile ("cli;hlt");
    }

    lapic_address = (void *)((uint64_t)madt->lapic_addr);

    paging_map(
        (void *)((uint64_t)lapic_address & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)lapic_address & 0xFFFFFFFFFFFFF000),
        (enum ptflag[]){ CacheDisabled, End }
    );

    write_msr(LAPIC_MSR, (read_msr(LAPIC_MSR) | 0x800) & ~(1 << 10));
    write_lapic_register(LAPIC_SINT,
        read_lapic_register(LAPIC_SINT) | (LAPIC_SINT_EN | 0xFF) | LAPIC_SINT_NO_BROADCAST_EOI);

    // Error status register
    write_lapic_register(LAPIC_ESR, 0);
    write_lapic_register(LAPIC_ESR, 0);

    // LVT error register
    write_lapic_register(LAPIC_LVT_ER, 0xFE | (1 << 8));

    // Disable LINT0 and LINT1 registers by masking
    write_lapic_register(LAPIC_LVT_LINT0, 0xFD | LAPIC_LVT_MASK);
    write_lapic_register(LAPIC_LVT_LINT1, 0xFD | LAPIC_LVT_MASK);

    write_lapic_register(LAPIC_TPR, 0);
    write_lapic_register(LAPIC_EOI, 0);

    printk("LAPIC initialized\n");
}