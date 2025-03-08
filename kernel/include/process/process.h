#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#include <stdint.h>
#include <mm/vmm/paging.h>
#include <utils/regs.h>

typedef struct process {
    regs_t regs;
    page_table_t *pml4;
    char fxsave_region[512] __attribute__((aligned(16)));
    uint32_t pid_id;
    struct process *next;
} process_t;

void process_lock();
void process_unlock();
void schedule(regs_t *regs);
void process_init();

#endif