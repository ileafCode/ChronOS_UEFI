BITS 64

__write_cr3:
    mov cr3, rdi
    ret
GLOBAL __write_cr3