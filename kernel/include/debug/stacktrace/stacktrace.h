#ifndef _KERNEL_STACKTRACE_H
#define _KERNEL_STACKTRACE_H

#include <stdint.h>

typedef struct stackframe {
    struct stackframe* rbp;
    uint64_t rip;
} stackframe_t;

void stacktrace_print();

#endif