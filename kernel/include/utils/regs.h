
#ifndef _KERNEL_REGS_H
#define _KERNEL_REGS_H

#include <stdint.h>

typedef struct regs {
    uint64_t r15, r14, r13, r12;
    uint64_t r11, r10,  r9,  r8;
    uint64_t rdi, rsi, rbp, rbx;
    uint64_t rdx, rcx, rax, rsp;
    uint64_t rip, cs,  rflags;
} __attribute__((packed)) regs_t;

typedef struct regs_err {
    uint64_t r15, r14, r13, r12;
    uint64_t r11, r10,  r9,  r8;
    uint64_t rdi, rsi, rbp, rbx;
    uint64_t rdx, rcx, rax, rsp;
    uint64_t erc, rip, cs,  rflags;
} __attribute__((packed)) regs_err_t;

typedef struct regs_ctx {
    uint64_t rax; // 0
    uint64_t rcx; // 8
    uint64_t rdx; // 16
    uint64_t rbx; // 24
    uint64_t rsp; // 32
    uint64_t rbp; // 40
    uint64_t rsi; // 48
    uint64_t rdi; // 56
    uint64_t rflags; // 64
    uint64_t cr3; // 72
    uint64_t rip; // 80
    uint64_t r8; // 80
    uint64_t r9; // 80
    uint64_t r10; // 80
    uint64_t r11; // 80
    uint64_t r12; // 80
    uint64_t r13; // 80
    uint64_t r14; // 80
    uint64_t r15; // 80
} __attribute__((packed)) regs_ctx_t;

#endif