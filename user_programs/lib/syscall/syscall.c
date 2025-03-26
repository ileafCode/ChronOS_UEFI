#include <syscall/syscall.h>

uint64_t syscall(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9) {
    uint64_t ret;
    
    asm volatile (
        "int $0x80"
        : "=a"(ret)                         // Output: return value in RAX
        : "a"(rax), "D"(rdi), "S"(rsi),     // Inputs: RAX, RDI, RSI
          "d"(rdx), "r"(rcx), "r"(r8), "r"(r9) // Additional registers
        : "rcx", "r11", "memory"            // Clobbers: RCX, R11, memory
    );

    return ret;
}