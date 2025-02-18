#include <mm/vmm/paging.h>
#include <mm/pmm/pmm.h>
#include <printk/printk.h>
#include <string/string.h>

void pde_setflag(uint64_t *pde, enum ptflag flag, int enabled) {
    uint64_t bitSelector = (uint64_t)1 << flag;
    *(pde) &= ~bitSelector;
    if (enabled){
        *(pde) |= bitSelector;
    }
}

int pde_getflag(uint64_t *pde, enum ptflag flag) {
    uint64_t bitSelector = (uint64_t)1 << flag;
    return *(pde) & bitSelector;
}

void pde_setaddress(uint64_t *pde, uint64_t address) {
    address &= 0x000000ffffffffff;
    *(pde) &= 0xfff0000000000fff;
    *(pde) |= (address << 12);
}

uint64_t pde_getaddress(uint64_t *pde) {
    return (*(pde) & 0x000ffffffffff000) >> 12;
}

void pmidx_init(page_map_idx_t *pmidx, uint64_t virtual) {
    virtual >>= 12;
    pmidx->P_i = virtual & 0x1ff;
    virtual >>= 9;
    pmidx->PT_i = virtual & 0x1ff;
    virtual >>= 9;
    pmidx->PD_i = virtual & 0x1ff;
    virtual >>= 9;
    pmidx->PDP_i = virtual & 0x1ff;
}

page_table_t *pml4;

void paging_change_dir(page_table_t *dir) {
    pml4 = dir;
    asm volatile ("mov %0, %%cr3" : : "r" (dir));
}

// Paging code (internal)

void __paging_map(void *virtual, void *physical, enum ptflag *flags) {
    page_map_idx_t indexer;
    pmidx_init(&indexer, (uint64_t)virtual);

    uint64_t pde;

    pde = pml4->entries[indexer.PDP_i];
    page_table_t *pdp;
    if (!pde_getflag(&pde, Present)) {
        pdp = (page_table_t *)pmm_getpage();
        memset(pdp, 0, 0x1000);
        pde_setaddress(&pde, (uint64_t)pdp >> 12);
        pde_setflag(&pde, Present, 1);
        pde_setflag(&pde, ReadWrite, 1);
        pde_setflag(&pde, CacheDisabled, 0);

        if (flags) {
            int i = 0;
            while (flags[i] != End) {
                pde_setflag(&pde, flags[i], 1);
                i++;
            }
        }

        pml4->entries[indexer.PDP_i] = pde;
    } else {
        pdp = (page_table_t *)((uint64_t)pde_getaddress(&pde) << 12);
    }

    pde = pdp->entries[indexer.PD_i];
    page_table_t *pd;
    if (!pde_getflag(&pde, Present)) {
        pd = (page_table_t *)pmm_getpage();
        memset(pd, 0, 0x1000);
        pde_setaddress(&pde, (uint64_t)pd >> 12);
        pde_setflag(&pde, Present, 1);
        pde_setflag(&pde, ReadWrite, 1);
        pde_setflag(&pde, CacheDisabled, 0);

        if (flags) {
            int i = 0;
            while (flags[i] != End) {
                pde_setflag(&pde, flags[i], 1);
                i++;
            }
        }

        pdp->entries[indexer.PD_i] = pde;
    } else {
        pd = (page_table_t *)((uint64_t)pde_getaddress(&pde) << 12);
    }

    pde = pd->entries[indexer.PT_i];
    page_table_t *pt;
    if (!pde_getflag(&pde, Present)) {
        pt = (page_table_t *)pmm_getpage();
        memset(pt, 0, 0x1000);
        pde_setaddress(&pde, (uint64_t)pt >> 12);
        pde_setflag(&pde, Present, 1);
        pde_setflag(&pde, ReadWrite, 1);
        pde_setflag(&pde, CacheDisabled, 0);

        if (flags) {
            int i = 0;
            while (flags[i] != End) {
                pde_setflag(&pde, flags[i], 1);
                i++;
            }
        }

        pd->entries[indexer.PT_i] = pde;
    } else {
        pt = (page_table_t *)((uint64_t)pde_getaddress(&pde) << 12);
    }

    pde = pt->entries[indexer.P_i];
    pde_setaddress(&pde, (uint64_t)physical >> 12);
    pde_setflag(&pde, Present, 1);
    pde_setflag(&pde, ReadWrite, 1);
    pde_setflag(&pde, CacheDisabled, 0);

    if (flags) {
        int i = 0;
        while (flags[i] != End) {
            pde_setflag(&pde, flags[i], 1);
            i++;
        }
    }

    pt->entries[indexer.P_i] = pde;
}

void __paging_unmap(void *virtual) {
    page_map_idx_t indexer;
    pmidx_init(&indexer, (uint64_t)virtual);

    uint64_t pde;

    // Get PDP (Page Directory Pointer Table)
    pde = pml4->entries[indexer.PDP_i];
    if (!pde_getflag(&pde, Present)) return; // Not mapped

    page_table_t *pdp = (page_table_t *)((uint64_t)pde_getaddress(&pde) << 12);

    // Get PD (Page Directory)
    pde = pdp->entries[indexer.PD_i];
    if (!pde_getflag(&pde, Present)) return;

    page_table_t *pd = (page_table_t *)((uint64_t)pde_getaddress(&pde) << 12);

    // Get PT (Page Table)
    pde = pd->entries[indexer.PT_i];
    if (!pde_getflag(&pde, Present)) return;

    page_table_t *pt = (page_table_t *)((uint64_t)pde_getaddress(&pde) << 12);

    // Get Page Table Entry (PTE) and clear it
    pde = pt->entries[indexer.P_i];
    if (!pde_getflag(&pde, Present)) return;

    pt->entries[indexer.P_i] = 0;
}

// Actual paging code that the kernel calls

void paging_map(void *virtual, void *physical, enum ptflag flags[]) {
    __paging_map(virtual, physical, flags);
    asm volatile ("invlpg (%0)" : : "b"(virtual) : "memory");
}

void paging_unmap(void *virtual) {
    __paging_unmap(virtual);
    asm volatile ("invlpg (%0)" : : "b"(virtual) : "memory");
}

void paging_init(boot_info_t *boot_info) {
    pml4 = pmm_getpage();
    memset(pml4, 0, 0x1000);

    uint64_t mMapEntries = boot_info->mMapSize / boot_info->mMapDescSize;

    for (uint64_t t = 0; t < get_mem_size(boot_info->mMap, mMapEntries, boot_info->mMapDescSize); t += 0x1000) {
        __paging_map((void *)t, (void *)t, NULL);
    }
    
    printk("Mapped memory\n");
    
    uint64_t fbBase = (uint64_t)boot_info->framebuffer->address;
    uint64_t fbSize = (uint64_t)boot_info->framebuffer->buf_size + 0x1000;
    for (uint64_t t = fbBase; t < fbBase + fbSize; t += 0x1000) {
        __paging_map((void *)t, (void *)t, NULL);
    }
    
    printk("Mapped framebuffer\n");
    
    asm volatile ("mov %0, %%cr3" : : "r"(pml4));
    printk("Paging initialized (CR3: 0x%x)\n", pml4);
}
