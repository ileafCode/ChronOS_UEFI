
#ifndef _KERNEL_SYSCALL_H
#define _KERNEL_SYSCALL_H

#include <stdint.h>
#include <utils/regs.h>

#define SYSCALL_TEST 0

void syscall(regs_t *regs);

#endif