#include <process/process.h>
#include <utils/panic.h>
#include <logging/logging.h>
#include <shmall_wrapper.h>
#include <string/string.h>
#include <mm/pmm/pmm.h>
#include <mm/vmm/vmm.h>
#include <io/io.h>

process_t *proc_queue = NULL;
process_t *cur_proc = NULL;
int num_pids = 0;

void schedule(regs_t *regs) {
    if (proc_queue == NULL) {
        //log_info("PROC", "Process queue is NULL.");
        return;
    }
    if (cur_proc == NULL) {
        log_info("PROC", "Current process is NULL.");
        return;
    }
    if (cur_proc->next == NULL) {
        log_info("PROC", "Next process is NULL.");
        return;
    }
    
    cur_proc->regs.rax = regs->rax;
    cur_proc->regs.rcx = regs->rcx;
    cur_proc->regs.rdx = regs->rdx;
    cur_proc->regs.rbx = regs->rbx;
    cur_proc->regs.rbp = regs->rbp;
    cur_proc->regs.rsp = regs->rsp;
    cur_proc->regs.rsi = regs->rsi;
    cur_proc->regs.rdi = regs->rdi;
    cur_proc->regs.rflags = regs->rflags;
    cur_proc->regs.rip = regs->rip;
    cur_proc->regs.r8  = regs->r8;
    cur_proc->regs.r9  = regs->r9;
    cur_proc->regs.r10 = regs->r10;
    cur_proc->regs.r11 = regs->r11;
    cur_proc->regs.r12 = regs->r12;
    cur_proc->regs.r13 = regs->r13;
    cur_proc->regs.r14 = regs->r14;
    cur_proc->regs.r15 = regs->r15;
    cur_proc->regs.cs  = regs->cs;
    asm volatile("fxsave %0 "::"m"(cur_proc->fxsave_region));

    cur_proc = cur_proc->next;

    regs->rax = cur_proc->regs.rax;
    regs->rcx = cur_proc->regs.rcx;
    regs->rdx = cur_proc->regs.rdx;
    regs->rbx = cur_proc->regs.rbx;
    regs->rbp = cur_proc->regs.rbp;
    regs->rsp = cur_proc->regs.rsp;
    regs->rsi = cur_proc->regs.rsi;
    regs->rdi = cur_proc->regs.rdi;
    regs->rflags = cur_proc->regs.rflags;
    regs->rip = cur_proc->regs.rip;
    regs->r8  = cur_proc->regs.r8;
    regs->r9  = cur_proc->regs.r9;
    regs->r10 = cur_proc->regs.r10;
    regs->r11 = cur_proc->regs.r11;
    regs->r12 = cur_proc->regs.r12;
    regs->r13 = cur_proc->regs.r13;
    regs->r14 = cur_proc->regs.r14;
    regs->r15 = cur_proc->regs.r15;
    regs->cs  = cur_proc->regs.cs;
    asm volatile("fxrstor %0 "::"m"(cur_proc->fxsave_region));
}

extern char _kern_start[];
extern char _kern_end[];

void make_proc(void *entry, int size_pages) {
    process_t *new_proc = (process_t *)pmm_getpage();
    memset(&new_proc->regs, 0, sizeof(regs_t));
    new_proc->regs.rflags = 0x202;
    new_proc->regs.rip = (uint64_t)entry;
    new_proc->regs.cs  = 0x08;

    // The stack
    new_proc->stack = pmm_getpage();
    new_proc->regs.rsp = (uint64_t)(new_proc->stack) + 0x1000;
    new_proc->regs.rbp = new_proc->regs.rsp;

    // Paging
    new_proc->pml4 = create_page_table();

    // Other stuff
    new_proc->pid_id = num_pids++;

    new_proc->bitmap->bitmap = pmm_getpage();
    memset(new_proc->bitmap->bitmap, 0, 0x1000);
    new_proc->bitmap->size = VMM_SIZE_PAGES / 8;

    memset(new_proc->fxsave_region, 0, 512);

    for (int i = 0; i < MAX_FILES_OPENABLE; i++) {
        new_proc->files[i] = NULL;
    }

    process_t *tmp_next = proc_queue->next;
    new_proc->next = tmp_next;
    proc_queue->next = new_proc;
}

void make_proc_from_elf(void *elf_data) {
    process_t *new_proc = (process_t *)pmm_getpage();
    memset(&new_proc->regs, 0, sizeof(regs_t));
    new_proc->regs.rflags = 0x202;
    new_proc->regs.cs  = 0x08;

    // The stack
    new_proc->stack = pmm_getpage();
    new_proc->regs.rsp = (uint64_t)(new_proc->stack) + 0x1000;
    new_proc->regs.rbp = new_proc->regs.rsp;

    // Paging
    new_proc->pml4 = create_page_table();
    __paging_map(new_proc->pml4, new_proc->stack, new_proc->stack, PAGE_NORMAL);

    new_proc->elf_program = load_elf(elf_data, new_proc->pml4);
    new_proc->regs.rip = (uint64_t)new_proc->elf_program->entry;
    new_proc->pid_id = num_pids++;

    memset(new_proc->fxsave_region, 0, 512);

    new_proc->bitmap->bitmap = pmm_getpage();
    memset(new_proc->bitmap->bitmap, 0, 0x1000);

    new_proc->bitmap->size = VMM_SIZE_PAGES / 8;

    for (int i = 0; i < MAX_FILES_OPENABLE; i++) {
        new_proc->files[i] = NULL;
    }

    process_t *tmp_next = proc_queue->next;
    new_proc->next = tmp_next;
    proc_queue->next = new_proc;
}

void idle() {
    while (1) {}
}

void process_set_pml4_to_kernel() {
    if (!proc_queue && !proc_queue->pml4)
        return;
    paging_change_dir(proc_queue->pml4);
}

void process_set_pml4_to_cur_proc() {
    if (!cur_proc)
        return;
    paging_change_dir(cur_proc->pml4);
}

process_t *get_cur_proc() {
    return cur_proc;
}

void process_init() {
    proc_queue = (process_t *)pmm_getpage();
    cur_proc = proc_queue;
    cur_proc->pid_id = num_pids++;
    cur_proc->pml4 = paging_get_pml4();
    cur_proc->next = cur_proc;

    make_proc((void *)idle, 0);
    log_ok("PROC", "Initialized processes");
}