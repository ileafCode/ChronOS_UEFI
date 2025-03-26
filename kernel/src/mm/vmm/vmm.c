#include <mm/vmm/vmm.h>
#include <mm/vmm/paging.h>
#include <logging/logging.h>

void *mmap(process_t *proc, void *phys, uint64_t flags, int pages) {
    // Sanity checks
    if (!phys || !pages || !proc) {
        return NULL;
    }

    flags |= PAGE_PRESENT;

    // Clear bottom 12 bits to make sure the address
    // is page aligned
    uint64_t phys_addr = (uint64_t)phys & 0xFFFFFFFFFFFFF000;

    // Look for free pages
    int idx = 0;
    while (1) {
        int x = 0;
        for (int i = 0; i < pages; i++) {
            if (bitmap_get(proc->bitmap, idx + i) == 0) {
                x++;
            }
        }

        if (x == pages) {
            break;
        }
        idx++;
    }

    void *virtaddr = (void *)((uint64_t)(VMM_ADDR_START) + (0x1000 * idx));
    log_info("VMM", "mmap - virtaddr = %lx, physaddr = %lx, flags = %d, idx = %d", virtaddr, phys_addr, flags, idx);

    for (int i = 0; i < pages; i++) {
        bitmap_set(proc->bitmap, idx + i, 1);
        __paging_map(proc->pml4, (void *)((uint64_t)virtaddr + (0x1000 * i)), (void *)(phys_addr + (0x1000 * i)), flags);
    }

    return virtaddr;
}

int munmap(process_t *proc, void *addr, int pages) {
    // Sanity checks
    if (!addr || !pages || !proc) {
        return -1;
    }

    int idx = ((uint64_t)(addr) - (uint64_t)(VMM_ADDR_START)) / 0x1000;
    for (int i = 0; i < pages; i++) {
        bitmap_set(proc->bitmap, idx + i, 0);
        __paging_unmap(proc->pml4, addr + (0x1000 * i));
    }
    log_info("VMM", "munmap - virtaddr = %lx, idx = %d", addr, idx);

    return 0;
}