#include <mm/vmm/paging.h>
#include <mm/pmm/pmm.h>
#include <logging/logging.h>
#include <string/string.h>

extern void __write_cr3(uint64_t);

void pde_setaddress(uint64_t *pde, uint64_t address) {
    address &= 0x000000ffffffffff;
    *(pde) &= 0xfff0000000000fff;
    *(pde) |= (address);
}

uint64_t pde_getaddress(uint64_t *pde) {
    return (*(pde) & 0x000ffffffffff000);
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

page_table_t *paging_get_pml4() {
    return pml4;
}

void paging_change_dir(page_table_t *dir) {
    pml4 = dir;
    __write_cr3((uint64_t)dir);
    //asm volatile ("mov %0, %%cr3" : : "r" ((uint64_t)dir));
}

// Paging code (internal)

void __paging_map(page_table_t *dir, void *virtual, void *physical, uint64_t flags) {
    page_map_idx_t indexer;
    pmidx_init(&indexer, (uint64_t)virtual);

    uint64_t pde;

    pde = dir->entries[indexer.PDP_i];
    page_table_t *pdp;
    if (!(pde & PAGE_PRESENT)) {
        pdp = (page_table_t *)pmm_getpage();
        memset(pdp, 0, 0x1000);
        pde_setaddress(&pde, (uint64_t)pdp);
        pde |= flags;

        dir->entries[indexer.PDP_i] = pde;
    } else {
        pdp = (page_table_t *)((uint64_t)pde_getaddress(&pde));
    }

    pde = pdp->entries[indexer.PD_i];
    page_table_t *pd;
    if (!(pde & PAGE_PRESENT)) {
        pd = (page_table_t *)pmm_getpage();
        memset(pd, 0, 0x1000);
        pde_setaddress(&pde, (uint64_t)pd);
        pde |= flags;

        pdp->entries[indexer.PD_i] = pde;
    } else {
        pd = (page_table_t *)((uint64_t)pde_getaddress(&pde));
    }

    pde = pd->entries[indexer.PT_i];
    page_table_t *pt;
    if (!(pde & PAGE_PRESENT)) {
        pt = (page_table_t *)pmm_getpage();
        memset(pt, 0, 0x1000);
        pde_setaddress(&pde, (uint64_t)pt);
        pde |= flags;

        pd->entries[indexer.PT_i] = pde;
    } else {
        pt = (page_table_t *)((uint64_t)pde_getaddress(&pde));
    }

    pde = pt->entries[indexer.P_i];
    pde_setaddress(&pde, (uint64_t)physical);
    pde |= flags;

    pt->entries[indexer.P_i] = pde;
}

void __paging_unmap(page_table_t *dir, void *virtual) {
    page_map_idx_t indexer;
    pmidx_init(&indexer, (uint64_t)virtual);

    uint64_t pde;

    // Get PDP (Page Directory Pointer Table)
    pde = dir->entries[indexer.PDP_i];
    if (!(pde & PAGE_PRESENT)) return; // Not mapped

    page_table_t *pdp = (page_table_t *)((uint64_t)pde_getaddress(&pde));

    // Get PD (Page Directory)
    pde = pdp->entries[indexer.PD_i];
    if (!(pde & PAGE_PRESENT)) return;

    page_table_t *pd = (page_table_t *)((uint64_t)pde_getaddress(&pde));

    // Get PT (Page Table)
    pde = pd->entries[indexer.PT_i];
    if (!(pde & PAGE_PRESENT)) return;

    page_table_t *pt = (page_table_t *)((uint64_t)pde_getaddress(&pde));

    // Get Page Table Entry (PTE) and clear it
    pde = pt->entries[indexer.P_i];
    if (!(pde & PAGE_PRESENT)) return;

    pt->entries[indexer.P_i] = 0;
}

// Actual paging code that the kernel calls

void paging_map(void *virtual, void *physical, uint64_t flags) {
    __paging_map(pml4, virtual, physical, flags);
    asm volatile ("invlpg (%0)" : : "b"(virtual) : "memory");
}

void paging_unmap(void *virtual) {
    __paging_unmap(pml4, virtual);
    asm volatile ("invlpg (%0)" : : "b"(virtual) : "memory");
}

extern char _kern_start[];
extern char _kern_end[];

boot_info_t *b_info;

page_table_t *create_page_table() {
    // Making the page table
    page_table_t *tbl = pmm_getpage();
    memset(tbl, 0, 0x1000);

    // Getting all memory map entries
    uint64_t mMapEntries = b_info->mMapSize / b_info->mMapDescSize;

    // Mapping the kernel
    for (uint64_t t = (uint64_t)_kern_start;
        t < (uint64_t)_kern_end + 0x1000;
        t += 0x1000) {
        __paging_map(tbl, (void *)t, (void *)t, PAGE_NORMAL);
    }

    // Mapping everything else (for some reason when I don't map this it doesn't work...)
    for (uint64_t t = (uint64_t)_kern_end + 0x1000;
        t < (uint64_t)get_mem_size(b_info->mMap, mMapEntries, b_info->mMapDescSize);
        t += 0x1000) {
        __paging_map(tbl, (void *)(t), (void *)t, PAGE_NORMAL);
    }

    return tbl;
}

void paging_init(boot_info_t *boot_info) {
    b_info = boot_info;
    pml4 = pmm_getpage();
    memset(pml4, 0, 0x1000);
    log_info("PAGE", "Initialized kernel PML4 page");

    uint64_t mMapEntries = boot_info->mMapSize / boot_info->mMapDescSize;

    uint64_t t;
    for (t = 0; t < get_mem_size(boot_info->mMap, mMapEntries, boot_info->mMapDescSize); t += 0x1000) {
        __paging_map(pml4, (void *)t, (void *)t, PAGE_NORMAL);
    }
    
    log_info("PAGE", "Mapped memory");
    
    uint64_t fbBase = (uint64_t)boot_info->framebuffer->address;
    uint64_t fbSize = (uint64_t)boot_info->framebuffer->buf_size + 0x1000;
    for (uint64_t t = fbBase; t < fbBase + fbSize; t += 0x1000) {
        __paging_map(pml4, (void *)t, (void *)t, PAGE_NORMAL | PAGE_CACHE_DISABLED | PAGE_WRITE_THROUGH);
    }
    
    asm volatile ("mov %0, %%cr3" : : "r"(pml4));
    log_ok("PAGE", "Paging initialized (CR3: 0x%x)", pml4);
}