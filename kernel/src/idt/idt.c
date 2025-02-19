#include <idt/idt.h>
#include <idt/int_handlers.h>
#include <terminal/terminal.h>
#include <logging/logging.h>
#include <debug/stacktrace/stacktrace.h>

idtr_t idtr;
idt_entry_t idt_entries[256];

void idt_entry_set_offset(idt_entry_t *entry, uint64_t offset){
    entry->offset0 = (uint16_t)(offset & 0x000000000000ffff);
    entry->offset1 = (uint16_t)((offset & 0x00000000ffff0000) >> 16);
    entry->offset2 = (uint32_t)((offset & 0xffffffff00000000) >> 32);
}

uint64_t idt_entry_get_offset(idt_entry_t *entry){ 
    uint64_t offset = 0;
    offset |= (uint64_t)entry->offset0;
    offset |= (uint64_t)entry->offset1 << 16;
    offset |= (uint64_t)entry->offset2 << 32;
    return offset;
}

void idt_set_gate(void *handler, uint8_t entryOffset, uint8_t type_attr, uint8_t selector) {
    idt_entry_t *interrupt = &idt_entries[entryOffset];
    idt_entry_set_offset(interrupt, (uint64_t)handler);
    interrupt->type_attr = type_attr;
    interrupt->selector = selector;
}

void idt_init() {
    idtr.limit = 0x0FFF;
    idtr.offset = (uint64_t)(&idt_entries);

    int_handlers_init();

    asm("lidt %0" : : "m"(idtr));
    asm volatile ("sti");

    log_ok("IDT", "IDT initialized\n");
}