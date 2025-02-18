[bits 64]
enable_optimizations:
    ; Enable FPU and SSE
    mov rax, cr0
    and rax, ~(1 << 2)
    or  rax, (1 << 1)
    mov cr0, rax

    ; Enable FXSAVE/FXSTOR
    mov rax, cr4
    or  rax, (1 << 9)
    mov cr4, rax
    ret
GLOBAL enable_optimizations