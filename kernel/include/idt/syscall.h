
#ifndef _KERNEL_SYSCALL_H
#define _KERNEL_SYSCALL_H

#include <stdint.h>
#include <utils/regs.h>

#define MAX_SYSCALLS 128

#define SYSCALL_TEST 0

typedef void (*syscall_t)(regs_t *);

void syscall(regs_t *regs);

#endif