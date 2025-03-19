
#ifndef _KERNEL_PAGING_H
#define _KERNEL_PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <bootinfo.h>

#define PAGE_PRESENT (1 << 0)
#define PAGE_READWRITE (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_WRITE_THROUGH (1 << 3)
#define PAGE_CACHE_DISABLED (1 << 4)
#define PAGE_ACCESSED (1 << 5)
#define PAGE_LARGE (1 << 7)
#define PAGE_NX (1 << 63)

#define PAGE_NORMAL PAGE_PRESENT | PAGE_READWRITE

typedef struct page_table { 
    uint64_t entries[512];
} page_table_t __attribute__((aligned(0x1000)));

typedef struct page_map_idx {
    uint64_t PDP_i;
    uint64_t PD_i;
    uint64_t PT_i;
    uint64_t P_i;
} page_map_idx_t;

/// @brief Sets the address
/// @param pde Pointer to the PDE
/// @param address The address
void pde_setaddress(uint64_t *pde, uint64_t address);

/// @brief Gets the address of the PDE
/// @param pde Pointer to the PDE
/// @return The address
uint64_t pde_getaddress(uint64_t *pde);

void pmidx_init(page_map_idx_t *pmidx, uint64_t virtual);

/// @brief Change the current page directory (PML4)
/// @param dir PML4 directory
void paging_change_dir(page_table_t *dir);

/// @brief Gets current PML4 directory
/// @return PML4 dir
page_table_t *paging_get_pml4();

void __paging_map(page_table_t *dir, void *virtual, void *physical, uint64_t flags);
void __paging_unmap(page_table_t *dir, void *virtual);

/// @brief Maps a virtual address to a physical address
/// @param virtual The virtual address
/// @param physical The physical address
/// @param flags Flags for the page as an array (make sure to end the array with the "End" enum, or just set it to NULL)
void paging_map(void *virtual, void *physical, uint64_t flags);

/// @brief Un-maps a virtual address
/// @param virtual The virtual address
void paging_unmap(void *virtual);

page_table_t *create_page_table();

void paging_init(boot_info_t *boot_info);

#endif