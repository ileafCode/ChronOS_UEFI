#include <timers/hpet/hpet.h>
#include <printk/printk.h>
#include <mm/vmm/paging.h>

// TODO

volatile hpet_acpi_table_t *hpet_table;
volatile uint64_t ticks_per_10us = 0;

void hpet_write(uint64_t reg, uint64_t value) {
    *(uint64_t *)(hpet_table->base_address.address + reg) = value;
}

uint64_t hpet_read(uint64_t reg) {
    return *(uint64_t *)(hpet_table->base_address.address + reg);
}

int hpet_init() {
    hpet_table = (hpet_acpi_table_t *)acpi_find_table("HPET");
    if (!hpet_table) {
        printk("HPET not found.");
        return 1;
    }

    paging_map(
        (void *)((uint64_t)hpet_table->base_address.address & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)hpet_table->base_address.address & 0xFFFFFFFFFFFFF000),
        (enum ptflag[]){ CacheDisabled, End }
    );

    printk("HPET initialized\n");
    return 0;
}