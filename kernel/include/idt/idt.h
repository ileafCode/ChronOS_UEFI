
#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <stdint.h>

#define IDT_TA_InterruptGate    0b10001110
#define IDT_TA_CallGate         0b10001100
#define IDT_TA_TrapGate         0b10001111

typedef struct idt_entry {
    uint16_t offset0; 
    uint16_t selector;  
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset1;
    uint32_t offset2;
    uint32_t ignore;
} __attribute__((packed)) idt_entry_t;

typedef struct idtr {
    uint16_t limit;
    uint64_t offset;
} __attribute__((packed)) idtr_t;

void idt_set_gate(void *handler, uint8_t entryOffset, uint8_t type_attr, uint8_t selector);
void idt_init();

#endif