#include <idt/syscall.h>
#include <printk/printk.h>

void syscall(regs_t *regs) {
    uint64_t syscall_num = regs->rax;
    switch (syscall_num) {
        case SYSCALL_TEST:
            regs->rbx += 10;
            printk("SYSCALL! RBX is now %d\n", regs->rbx);
            break;
    }
}