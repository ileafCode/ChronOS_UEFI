
#ifndef _KERNEL_HPET_H
#define _KERNEL_HPET_H

#include <stdint.h>
#include <acpi/acpi.h>

typedef struct hpet_address {
    uint8_t addr_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t resv1;
    uint64_t address;
} __attribute__((packed)) hpet_address_t;

typedef struct hpet_acpi_table {
    sdt_hdr_t h;
    uint32_t event_timer_block_id;
    hpet_address_t base_address;
    uint8_t hpet_num;
    uint16_t min_clock_tick;
} __attribute__((packed)) hpet_acpi_table_t;

/// @brief Initiializes the HPET.
/// @return ret == 0 - ok, ret == 1 - no HPET.
int hpet_init();

#endif