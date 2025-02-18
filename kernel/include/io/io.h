
#ifndef _KERNEL_IO_H
#define _KERNEL_IO_H

#include <stdint.h>

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);

void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);

uint64_t read_msr(uint32_t msr);
void write_msr(uint32_t msr, uint64_t value);

#endif