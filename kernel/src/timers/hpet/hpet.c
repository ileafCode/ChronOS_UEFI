#include <timers/hpet/hpet.h>
#include <logging/logging.h>
#include <mm/vmm/paging.h>
#include <apic/ioapic.h>

volatile hpet_acpi_table_t *hpet_table;
volatile uint64_t ticks_per_10us = 0;

void hpet_write(uint64_t reg, uint64_t value) {
    *(uint64_t *)(hpet_table->base_address.address + reg) = value;
}

uint64_t hpet_read(uint64_t reg) {
    return *(uint64_t *)(hpet_table->base_address.address + reg);
}

void hpet_usleep(uint64_t us) {
    if (us < 10)
        us = 10;

    uint64_t ticks_to_wait = (us / 10) * ticks_per_10us;
    volatile uint64_t now = hpet_read(HPET_MAIN_COUNTER);

    while ((hpet_read(HPET_MAIN_COUNTER) - now) < ticks_to_wait);
}

uint64_t hpet_get_ticks() {
    return hpet_read(HPET_MAIN_COUNTER);
}

int hpet_init() {
    hpet_table = (hpet_acpi_table_t *)acpi_find_table("HPET");
    if (!hpet_table) {
        log_warn("HPET", "HPET not found.");
        return 1;
    }
    log_info("HPET", "Found HPET ACPI table");

    paging_map(
        (void *)((uint64_t)hpet_table->base_address.address & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)hpet_table->base_address.address & 0xFFFFFFFFFFFFF000),
        PAGE_NORMAL | PAGE_CACHE_DISABLED
    );
    log_info("HPET", "Mapped HPET address to virtual memory");

    uint32_t clk_period_fs = (hpet_read(HPET_GENERAL_CAPABILITIES_REG) >> 32) & 0xFFFFFFFF;
    uint64_t hpet_freq_hz = HPET_ONE_FS / clk_period_fs;
    ticks_per_10us = hpet_freq_hz / 100000;
    log_info("HPET", "Calculated the amount of ticks per 10us");

    // Disable and reset the HPET, if it isn't already
    hpet_write(HPET_GENERAL_CONFIGURATION_REG, HPET_GEN_CFG_LEG_RT);
    hpet_write(HPET_MAIN_COUNTER, 0);
    log_info("HPET", "Disabled and reset the HPET");

    // Configure Timer 0
    hpet_write(HPET_TIMER0_CONFIGURATION_CAP_REG,
        HPET_TIM_CFG_CAP_INT |
        HPET_TIM_CFG_CAP_PERIODIC);
    hpet_write(HPET_TIMER0_COMPARATOR_REG, ticks_per_10us);
    log_info("HPET", "Configured Timer 0");

    // Enable the HPET
    hpet_write(HPET_GENERAL_CONFIGURATION_REG, HPET_GEN_CFG_EN_CNF | HPET_GEN_CFG_LEG_RT);
    log_info("HPET", "Enabled HPET");

    ioapic_set_entry(ioapic_remap_irq(0), 0x20);

    log_ok("HPET", "HPET initialized");
    return 0;
}