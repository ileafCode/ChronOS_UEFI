
#ifndef _KERNEL_HPET_H
#define _KERNEL_HPET_H

#include <stdint.h>
#include <acpi/acpi.h>

#define HPET_ONE_FS 1e15 // One femtosecond

#define HPET_GENERAL_CAPABILITIES_REG  0x000
#define HPET_GENERAL_CONFIGURATION_REG 0x010
 
#define HPET_MAIN_COUNTER 0x0F0

#define HPET_TIMER0_CONFIGURATION_CAP_REG 0x100
#define HPET_TIMER0_COMPARATOR_REG 0x108

#define HPET_GEN_CFG_EN_CNF (1 << 0)
#define HPET_GEN_CFG_LEG_RT (1 << 1)

#define HPET_TIM_CFG_CAP_INT (1 << 2)
#define HPET_TIM_CFG_CAP_PERIODIC (1 << 3)
#define HPET_TIM_CFG_CAP_32BIT (1 << 8)

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

/// @brief Sleeps for x microseconds
/// @param us Microseconds
void hpet_usleep(uint64_t us);

/// @brief Gets ticks since HPET is on
/// @return Ticks
uint64_t hpet_get_ticks();

/// @brief Initiializes the HPET.
/// @return ret == 0 - ok, ret == 1 - no HPET.
int hpet_init();

#endif