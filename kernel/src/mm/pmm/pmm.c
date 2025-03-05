#include <mm/pmm/pmm.h>
#include <terminal/terminal.h>
#include <logging/logging.h>
#include <string/string.h>

typedef struct mem_block {
    void *address;
    uint64_t size;
} mem_block_t;

// 0 - FREE
// 1 - USED
uint8_t *bitmap;
uint64_t bitmap_size = 0;

volatile void *start_of_mem = NULL;

mem_block_t *mem_blocks = (mem_block_t*)0x1F000;
int num_mem_blocks = 0;

extern uint64_t _kern_start;
extern uint64_t _kern_end;

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

void pmm_lockpages(void *addr, int pages);

void pmm_init(EFI_MEMORY_DESCRIPTOR *mmap, uint64_t mmap_size, uint64_t mmap_descsize) {
    uint64_t mMapEntries = mmap_size / mmap_descsize;

    void *largestFreeMemSeg = NULL;
    uint64_t largestFreeMemSegSize = 0;

    for (int i = 0; i < mMapEntries; i++) {
        EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((uint64_t)mmap + (i * mmap_descsize));
        if (desc->type == 7) { // type = EfiConventionalMemory
            mem_block_t blk = { 
                (void *)desc->physAddr,
                desc->numPages * 4096
            };
            mem_blocks[num_mem_blocks++] = blk;
        }
    }

    for (int i = 0; i < num_mem_blocks; i++) {
        if (mem_blocks[i].size * 4096 > largestFreeMemSegSize) {
            largestFreeMemSeg = (void *)((uint64_t)(mem_blocks[i].address) + 0x4000);
            largestFreeMemSegSize = (mem_blocks[i].size * 0x1000) - 0x4000;
            bitmap = (uint8_t *)((uint64_t)(largestFreeMemSeg) - 0x4000);
        }
    }

    memset(bitmap, 0, 0x4000);

    start_of_mem = largestFreeMemSeg;
    log_info("PMM", "Located largest free memory segment");

    uint64_t memory_size = get_mem_size(mmap, mMapEntries, mmap_descsize);
    bitmap_size = ((memory_size / 4096) / 8) + 1;
    log_info("PMM", "Got memory and bitmap size");

    log_ok("PMM", "PMM initialized");

    if ((uint64_t)start_of_mem < _kern_start) {
        uint64_t kernel_size = _kern_end - _kern_start;
        uint64_t kern_size_pages = (kernel_size / 0x1000) + 1;
        pmm_lockpages((void *)_kern_start, kern_size_pages);
    }
}

void *pmm_getpage() {
    int idx = 0;
    while (bitmap_get(idx) != 0) {
        idx++;
    }
    bitmap_set(idx, 1);
    //lastAllocatedPage = idx;

    void *pageaddr = (void *)((uint64_t)(start_of_mem) + (0x1000 * idx));
    //printk("%lx\n", pageaddr);
    return pageaddr;
}

void pmm_freepage(void *pageaddr) {
    int idx = ((uint64_t)(pageaddr) - (uint64_t)(start_of_mem)) / 0x1000;
    bitmap_set(idx, 0);
}

void pmm_lockpages(void *addr, int pages) {
    int idx = ((uint64_t)(addr) - (uint64_t)(start_of_mem)) / 0x1000;
    for (int i = 0; i < pages; i++) {
        bitmap_set(idx + i, 1);
    }
}
