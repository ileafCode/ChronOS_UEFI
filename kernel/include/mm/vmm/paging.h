
#ifndef _KERNEL_PAGING_H
#define _KERNEL_PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <bootinfo.h>

enum ptflag {
    Present = 0,
    ReadWrite = 1,
    UserSuper = 2,
    WriteThrough = 3,
    CacheDisabled = 4,
    Accessed = 5,
    LargerPages = 7,
    Custom0 = 9,
    Custom1 = 10,
    Custom2 = 11,
    NX = 63, // only if supported
    End = 100
};

typedef struct page_table { 
    uint64_t entries[512];
} page_table_t __attribute__((aligned(0x1000)));

typedef struct page_map_idx {
    uint64_t PDP_i;
    uint64_t PD_i;
    uint64_t PT_i;
    uint64_t P_i;
} page_map_idx_t;

/// @brief Sets a flag of PDE
/// @param pde Pointer to the PDE
/// @param flag What flag to set
/// @param enabled Value to set to (0 - off, 1 - on)
void pde_setflag(uint64_t *pde, enum ptflag flag, int enabled);

/// @brief Gets a flag of PDE
/// @param pde Pointer to the PDE
/// @param flag  What flag to get from the PDE
/// @return The flag
int pde_getflag(uint64_t *pde, enum ptflag flag);

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

/// @brief Maps a virtual address to a physical address
/// @param virtual The virtual address
/// @param physical The physical address
/// @param flags Flags for the page as an array (make sure to end the array with the "End" enum, or just set it to NULL)
void paging_map(void *virtual, void *physical, enum ptflag flags[]);

/// @brief Un-maps a virtual address
/// @param virtual The virtual address
void paging_unmap(void *virtual);

void paging_init(boot_info_t *boot_info);

#endif