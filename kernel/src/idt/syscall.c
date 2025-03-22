#include <idt/syscall.h>
#include <printk/printk.h>
#include <mm/vmm/vmm.h>

/*
    rax	      System call number
    rdi	      First argument
    rsi	      Second argument
    rdx	      Third argument
    r10	      Fourth argument
    r8	      Fifth argument
    r9	      Sixth argument
    rax (out) Return value
*/

void test_syscall(regs_t *regs) {
    regs->rbx += 10;
    printk("SYSCALL! RBX is now %d\n", regs->rbx);
}

void mmap_syscall(regs_t *regs) {
    process_t *proc = get_cur_proc();
    void *ret = mmap(proc, (void *)regs->rdi, regs->rsi, (int)regs->rdx);
    regs->rax = (uint64_t)ret;
}

syscall_t syscalls[MAX_SYSCALLS] = {
    test_syscall,
    mmap_syscall,
};

void syscall(regs_t *regs) {
    uint64_t syscall_num = regs->rax;
    syscall_t syscall_handler = syscalls[syscall_num];

    if (syscall_handler) {
        syscall_handler(regs);
    }
}