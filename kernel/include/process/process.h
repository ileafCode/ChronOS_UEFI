#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#include <stdint.h>
#include <mm/vmm/paging.h>
#include <utils/regs.h>
#include <loader/elf.h>
#include <ds/bitmap.h>

typedef struct process {
    regs_t regs;
    page_table_t *pml4;
    char fxsave_region[512] __attribute__((aligned(16)));
    void *stack;
    uint32_t pid_id;
    elf_prg_t *elf_program;
    // vmm stuff
    bitmap_t *bitmap;
    struct process *next;
} process_t;

void make_proc_from_elf(void *elf_data);

void schedule(regs_t *regs);
void process_set_pml4_to_kernel();
void process_set_pml4_to_cur_proc();
process_t *get_cur_proc();
void process_init();

#endif