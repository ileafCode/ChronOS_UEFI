#ifndef _KERNEL_PMM_H
#define _KERNEL_PMM_H

#include <stdint.h>
#include <stddef.h>
#include <efimem.h>

#define EfiReservedMemoryType 0
#define EfiLoaderCode 1
#define EfiLoaderData 2
#define EfiBootServicesCode 3
#define EfiBootServicesData 4
#define EfiRuntimeServicesCode 5
#define EfiRuntimeServicesData 6
#define EfiConventionalMemory 7
#define EfiUnusableMemory 8
#define EfiACPIReclaimMemory 9
#define EfiACPIMemoryNVS 10
#define EfiMemoryMappedIO 11
#define EfiMemoryMappedIOPortSpace 12
#define EfiPalCode 13

uint64_t get_mem_size(EFI_MEMORY_DESCRIPTOR *mmap, uint64_t mmap_entries, uint64_t mmap_descsize);
void pmm_init(EFI_MEMORY_DESCRIPTOR *mmap, uint64_t mmap_size, uint64_t mmap_descsize);
void *pmm_getpage();
void *pmm_getpages(int pages);
void pmm_freepage(void *pageaddr);

#endif