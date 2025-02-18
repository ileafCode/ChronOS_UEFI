#include <mm/pmm/pmm.h>
#include <terminal/terminal.h>
#include <printk/printk.h>

// 0 - FREE
// 1 - USED
uint8_t *bitmap = (uint8_t*)0x20000;
uint64_t bitmap_size = 0;

void *start_of_mem = NULL;

uint8_t bitmap_get(uint32_t idx) {
    if (idx > bitmap_size * 8)
        return 0;
    uint64_t byteIndex = idx / 8;
    uint8_t bitIndex = idx % 8;
    uint8_t bitIndexer = 0b10000000 >> bitIndex;
    if ((bitmap[byteIndex] & bitIndexer) > 0)
        return 1;
    return 0;
}

uint8_t bitmap_set(uint32_t idx, uint8_t value) {
    if (idx > bitmap_size * 8)
        return 0;
    uint64_t byteIndex = idx / 8;
    uint8_t bitIndex = idx % 8;
    uint8_t bitIndexer = 0b10000000 >> bitIndex;
    bitmap[byteIndex] &= ~bitIndexer;
    if (value)
        bitmap[byteIndex] |= bitIndexer;
    return 1;
}

uint64_t get_mem_size(EFI_MEMORY_DESCRIPTOR *mmap, uint64_t mmap_entries, uint64_t mmap_descsize) {
    static uint64_t memorySizeBytes = 0;
    if (memorySizeBytes > 0)
        return memorySizeBytes;

    for (int i = 0; i < mmap_entries; i++) {
        EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((uint64_t)mmap + (i * mmap_descsize));
        memorySizeBytes += desc->numPages * 4096;
    }

    return memorySizeBytes;
}

void pmm_init(EFI_MEMORY_DESCRIPTOR *mmap, uint64_t mmap_size, uint64_t mmap_descsize) {
    uint64_t mMapEntries = mmap_size / mmap_descsize;

    void *largestFreeMemSeg = NULL;
    uint64_t largestFreeMemSegSize = 0;

    for (int i = 0; i < mMapEntries; i++) {
        EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((uint64_t)mmap + (i * mmap_descsize));
        if (desc->type == 7) { // type = EfiConventionalMemory
            if (desc->numPages * 4096 > largestFreeMemSegSize) {
                largestFreeMemSeg = desc->physAddr;
                largestFreeMemSegSize = desc->numPages * 4096;
            }
        }
    }

    start_of_mem = largestFreeMemSeg;
    printk("Located largest free memory segment\n");

    uint64_t memory_size = get_mem_size(mmap, mMapEntries, mmap_descsize);
    bitmap_size = ((memory_size / 4096) / 8) + 1;

    printk("PMM initialized\n");
}

void *pmm_getpage() {
    int idx = 0;
    while (bitmap_get(idx) != 0) {
        idx++;
    }
    bitmap_set(idx, 1);
    //lastAllocatedPage = idx;

    void *pageaddr = (void *)((uint64_t)(start_of_mem) + (0x1000 * idx));
    return pageaddr;
}

void pmm_freepage(void *pageaddr) {
    int idx = ((uint64_t)(pageaddr) - (uint64_t)(start_of_mem)) / 0x1000;
    bitmap_set(idx, 0);
}
