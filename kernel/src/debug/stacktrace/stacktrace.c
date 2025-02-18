#include <debug/stacktrace/stacktrace.h>
#include <printk/printk.h>

void stacktrace_print() {
    stackframe_t *stk;
    asm volatile ("movq %%rbp,%0" : "=r"(stk) ::);
    printk(" --- Stack trace ---\n");
    for (uint64_t frame = 0; stk; ++frame) {
        printk("    %d: 0x%x\n", frame, stk->rip);
        stk = stk->rbp;
    }
}