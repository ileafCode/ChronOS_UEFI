
#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stdint.h>

typedef struct gdt_desc {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) gdt_desc_t;

typedef struct gdt_entry {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t limit1_flags;
    uint8_t base2;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt {
    gdt_entry_t null; // 0x00
    gdt_entry_t kernel_code; // 0x08
    gdt_entry_t kernel_data; // 0x10
    gdt_entry_t user_null; // 0x18
    gdt_entry_t user_data; // 0x20
    gdt_entry_t user_code; // 0x28
    gdt_entry_t ovmf_data; // 0x30
    gdt_entry_t ovmf_code; // 0x38
    gdt_entry_t tss_low;  // 0x40
    gdt_entry_t tss_high; // 0x48
} __attribute__((packed)) gdt_t 
__attribute((aligned(0x1000)));

typedef struct tss {
    uint32_t reserved0; uint64_t rsp0;      uint64_t rsp1;
    uint64_t rsp2;      uint64_t reserved1; uint64_t ist1;
    uint64_t ist2;      uint64_t ist3;      uint64_t ist4;
    uint64_t ist5;      uint64_t ist6;      uint64_t ist7;
    uint64_t reserved2; uint16_t reserved3; uint16_t iopb_offset;
} tss_t;

void gdt_init();

#endif