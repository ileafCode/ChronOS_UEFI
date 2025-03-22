
#ifndef _KERNEL_VMM_H
#define _KERNEL_VMM_H

#include <stdint.h>
#include <stddef.h>
#include <process/process.h>
#include <ds/bitmap.h>

#define VMM_ADDR_START 0xFFFFFFFF00000000

void *mmap(process_t *proc, void *phys, uint64_t flags, int pages);

#endif