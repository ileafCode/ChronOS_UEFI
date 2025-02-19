
#ifndef _KERNEL_LAPIC_H
#define _KERNEL_LAPIC_H

#include <stdint.h>
#include <acpi/acpi.h>

// LAPIC MSR base
#define LAPIC_MSR 0x1B

// LAPIC ID Register
#define LAPIC_ID 0x20

// LAPIC Version Register
#define LAPIC_VER 0x30

// LAPIC Task Priority Register
#define LAPIC_TPR 0x80

// LAPIC End of Interrupt
#define LAPIC_EOI 0xB0

// LAPIC Spurious Interrupt Vector Register
#define LAPIC_SINT 0x00F0

// If set in the LAPIC Spurious Interrupt Vector Register, then EOI messages will
// not be broadcast
#define LAPIC_SINT_NO_BROADCAST_EOI (1 << 12)

// Set this in the LAPIC Spurious Interrupt Vector Register to enable the LAPIC
#define LAPIC_SINT_EN (1 << 8)

// LAPIC Spurious Interrupt Vector Register
#define LAPIC_ESR 0x280

// LAPIC LVT mask
#define LAPIC_LVT_MASK (1 << 16)

// LAPIC LVT Error Register
#define LAPIC_LVT_ER 0x370

// LAPIC Error Status Register
#define LAPIC_ESR 0x280

// LAPIC LVTs LINT0 and 1
#define LAPIC_LVT_LINT0 0x350
#define LAPIC_LVT_LINT1 0x360

int get_lapic_id();
uint32_t get_lapic_addr();
void lapic_init();

#endif