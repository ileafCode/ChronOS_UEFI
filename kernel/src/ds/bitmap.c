
#include <ds/bitmap.h>

uint8_t bitmap_get(bitmap_t *bitmap, uint32_t idx) {
    if (idx > bitmap->size * 8)
        return 0;
    uint64_t byteIndex = idx / 8;
    uint8_t bitIndex = idx % 8;
    uint8_t bitIndexer = 0b10000000 >> bitIndex;
    if ((bitmap->bitmap[byteIndex] & bitIndexer) > 0)
        return 1;
    return 0;
}

uint8_t bitmap_set(bitmap_t *bitmap, uint32_t idx, uint8_t value) {
    if (idx > bitmap->size * 8)
        return 0;
    uint64_t byteIndex = idx / 8;
    uint8_t bitIndex = idx % 8;
    uint8_t bitIndexer = 0b10000000 >> bitIndex;
    bitmap->bitmap[byteIndex] &= ~bitIndexer;
    if (value)
        bitmap->bitmap[byteIndex] |= bitIndexer;
    return 1;
}