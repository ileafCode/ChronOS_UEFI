
#ifndef _KERNEL_EFIMEM_H
#define _KERNEL_EFIMEM_H

#include <stdint.h>

typedef struct
{
    uint32_t type;
    void *physAddr;
    void *virtAddr;
    uint64_t numPages;
    uint64_t attribs;
} EFI_MEMORY_DESCRIPTOR;

#endif