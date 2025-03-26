#include <idt/syscall.h>
#include <logging/logging.h>
#include <mm/vmm/vmm.h>
#include <shmall_wrapper.h>
#include <process/process.h>
#include <string/string.h>

/*
    rax	      System call number
    rdi	      First argument
    rsi	      Second argument
    rdx	      Third argument
    rcx	      Fourth argument
    r8	      Fifth argument
    r9	      Sixth argument
    rax (out) Return value
*/

void test_syscall(regs_t *regs) {
    regs->rdi += 10;
    printk("SYSCALL! RDI is now %d\n", regs->rdi);
}

void mmap_syscall(regs_t *regs) {
    process_t *proc = get_cur_proc();
    void *ret = mmap(proc, (void *)regs->rdi, regs->rsi, (int)regs->rdx);
    regs->rax = (uint64_t)ret;
}

void munmap_syscall(regs_t *regs) {
    process_t *proc = get_cur_proc();
    int ret = munmap(proc, (void *)regs->rdi, (int)regs->rsi);
    regs->rax = (uint64_t)ret;
}

void open_syscall(regs_t *regs) {
    process_t *proc = get_cur_proc();

    // Ignoring 0 and 1 because they are for i/o
    int x = -1;
    for (int i = 2; i < MAX_FILES_OPENABLE; i++) {
        if (proc->files[i] != NULL) {
            continue;
        }
        x = i;
    }

    if (x == -1) {
        regs->rax = 1;
        return;
    }

    char fname[256];

    process_set_pml4_to_cur_proc();
    memcpy(fname, (const TCHAR *)regs->rdi, strlen((const TCHAR *)regs->rdi) + 1);
    process_set_pml4_to_kernel();

    proc->files[x] = kmalloc(sizeof(FIL));
    FRESULT res = f_open(proc->files[x], fname, (BYTE)regs->rsi);
    if (res) {
        regs->rax = -res;
        return;
    }
    regs->rax = x; // returns file descriptor
}

void close_syscall(regs_t *regs) {
    process_t *proc = get_cur_proc();
    if (proc->files[regs->rdi] == NULL) {
        regs->rax = -2;
        return;
    }
    FRESULT res = f_close(proc->files[regs->rdi]);
    if (res) {
        regs->rax = -res;
        return;
    }
    regs->rax = 0;
}

void read_syscall(regs_t *regs) {
    process_t *proc = get_cur_proc();
}

void write_syscall(regs_t *regs) {
    process_t *proc = get_cur_proc();

    int fd = regs->rdi;
    int len = regs->rdx;
    char buffer[len];

    //printk("%d %d %lx\n", fd, len, regs->rsi);

    process_set_pml4_to_cur_proc();
    memcpy(buffer, (const char *)regs->rsi, len);
    process_set_pml4_to_kernel();

    if (fd == 1) { // STDOUT
        int i;
        for (i = 0; i < len; i++) {
            terminal_putc(buffer[i]);
        }

        regs->rax = i;
        return;
    }
}

syscall_t syscalls[MAX_SYSCALLS] = {
    test_syscall,
    mmap_syscall,
    munmap_syscall,
    open_syscall,
    close_syscall,
    read_syscall,
    write_syscall,
};

void syscall(regs_t *regs) {
    uint64_t syscall_num = regs->rax;
    syscall_t syscall_handler = syscalls[syscall_num];

    if (syscall_handler) {
        syscall_handler(regs);
    }
}