
#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <stdint.h>
#include <heap.h>

void *kmalloc(int size);
void kfree(void *ptr);

void heap_init();

#endif