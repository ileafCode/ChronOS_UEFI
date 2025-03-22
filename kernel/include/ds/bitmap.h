
#ifndef _KERNEL_BITMAP_H
#define _KERNEL_BITMAP_H

#include <stdint.h>

typedef struct bitmap {
    uint8_t *bitmap;
    uint64_t size;
} bitmap_t;

uint8_t bitmap_get(bitmap_t *bitmap, uint32_t idx);
uint8_t bitmap_set(bitmap_t *bitmap, uint32_t idx, uint8_t value);

#endif