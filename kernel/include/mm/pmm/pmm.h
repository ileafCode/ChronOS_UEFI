#ifndef _KERNEL_PMM_H
#define _KERNEL_PMM_H

#include <stdint.h>
#include <stddef.h>
#include <efimem.h>

uint64_t get_mem_size(EFI_MEMORY_DESCRIPTOR *mmap, uint64_t mmap_entries, uint64_t mmap_descsize);
void pmm_init(EFI_MEMORY_DESCRIPTOR *mmap, uint64_t mmap_size, uint64_t mmap_descsize);
void *pmm_getpage();
void pmm_freepage(void *pageaddr);

#endif