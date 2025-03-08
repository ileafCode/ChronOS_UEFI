#include <process/process.h>
#include <utils/panic.h>
#include <logging/logging.h>
#include <shmall_wrapper.h>
#include <string/string.h>
#include <mm/pmm/pmm.h>
#include <io/io.h>

process_t *proc_queue = NULL;
process_t *cur_proc = NULL;
int num_pids = 0;
int locked = 0;

void process_lock() {
    locked = 1;
}

void process_unlock() {
    locked = 0;
}

void schedule(regs_t *regs) {
    if (locked)
        return;
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
    //memcpy(&cur_proc->regs, regs, sizeof(regs_t));
    asm volatile(" fxsave %0 "::"m"(cur_proc->fxsave_region));

    //printk("Current: %lx Next: %lx\n", regs->rip, cur_proc->next->regs.rip);

    //log_info("PROC", "cur_proc: %lx, next proc: %lx", cur_proc, cur_proc->next);
    cur_proc = cur_proc->next;
    //memcpy(regs, &cur_proc->regs, sizeof(regs_t));

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
    asm volatile("fxrstor %0 "::"m"(cur_proc->fxsave_region));
}

void make_proc(void *entry, int size_pages) {
    process_t *new_proc = (process_t *)pmm_getpage();//kmalloc(sizeof(process_t));
    new_proc->regs.rax = 0;
    new_proc->regs.rcx = 0;
    new_proc->regs.rdx = 0;
    new_proc->regs.rbx = 0;
    new_proc->regs.rbp = 0;
    new_proc->regs.rsi = 420;
    new_proc->regs.rdi = 69;
    new_proc->regs.rflags = 0x202;
    new_proc->regs.rip = (uint64_t)entry;
    new_proc->regs.r8  = 0;
    new_proc->regs.r9  = 0;
    new_proc->regs.r10 = 0;
    new_proc->regs.r11 = 0;
    new_proc->regs.r12 = 0;
    new_proc->regs.r13 = 0;
    new_proc->regs.r14 = 0;
    new_proc->regs.r15 = 0;

    //new_proc->regs.cr3 = (uint64_t)paging_get_pml4();
    new_proc->pml4 = paging_get_pml4();

    new_proc->regs.rsp = (uint64_t)(pmm_getpage()) + 0x1000;

    /*process_t *first_proc_in_queue = proc_queue;
    new_proc->next = first_proc_in_queue;
    proc_queue = new_proc;*/

    memset(new_proc->fxsave_region, 0, 512);

    process_t *tmp_next = proc_queue->next;
    new_proc->next = tmp_next;
    proc_queue->next = new_proc;
}

void idle(uint64_t x, uint64_t y) {
    //printk("X: %d, Y: %d\n", x, y);
    asm volatile (
        "movq $0,  %rax\n"
        "movq $69, %rbx\n"
        "int $0x80\n"
    );
    asm volatile (
        "movq $0,  %rax\n"
        "movq $69, %rbx\n"
        "int $0x80\n"
    );
    asm volatile (
        "movq $0,  %rax\n"
        "movq $69, %rbx\n"
        "int $0x80\n"
    );
    while (1) {
        //printk("Hi\n");
    }
}

void process_init() {
    proc_queue = (process_t *)pmm_getpage();//kmalloc(sizeof(process_t));
    cur_proc = proc_queue;
    cur_proc->next = cur_proc;

    make_proc((void *)idle, 0);
    log_ok("PROC", "Initialized processes");
}