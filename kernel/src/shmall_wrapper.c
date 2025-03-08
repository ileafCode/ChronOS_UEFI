
#include <shmall_wrapper.h>
#include <mm/pmm/pmm.h>
#include <string/string.h>
#include <logging/logging.h>

heap_t *heap;
void *region;

void *kmalloc(int size) {
    return heap_alloc(heap, size);
}

void kfree(void *ptr) {
    heap_free(heap, ptr);
}

void heap_init() {
    heap = pmm_getpage();
    memset(heap, 0, sizeof(heap_t));

    region = pmm_getpage();
    for (int i = 1; i < 0x200; i++) {
        pmm_getpage();
    }
    memset(region, 0, HEAP_INIT_SIZE);

    heap->bins[0] = pmm_getpage();
    memset(heap->bins[0], 0, sizeof(bin_t));

    for (int i = 1; i < BIN_COUNT; i++) {
        heap->bins[i] = heap->bins[0] + (sizeof(bin_t) * i);
    }

    init_heap(heap, (long)region);
}