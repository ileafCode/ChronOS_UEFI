#include <loader/elf.h>
#include <logging/logging.h>
#include <string/string.h>
#include <shmall_wrapper.h>
#include <mm/pmm/pmm.h>

int elf_check(Elf64_Ehdr *ehdr) {
    if (
        ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3
    ) {
        return 1;
    }
    return 0; // Good
}

elf_prg_t *load_elf(void *data, page_table_t *page_table) {
    asm volatile ("cli");
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *) data;
    if (elf_check(ehdr)) {
        log_error("ELF", "Incorrect header\n");
        return NULL;
    }

    uint64_t prog_size = 0;

    for (uint64_t i = 0; i < (uint64_t)ehdr->e_shnum * ehdr->e_shentsize; i += ehdr->e_shentsize) {
        Elf64_Shdr *shdr = (Elf64_Shdr *)((uint64_t)ehdr + (ehdr->e_shoff + i));
        prog_size += shdr->sh_size;
    }

    int prog_size_pages = (prog_size / 0x1000) + 1;

    elf_prg_t *prog = (elf_prg_t *)kmalloc(sizeof(elf_prg_t));
    prog->entry = ehdr->e_entry;
    prog->program = pmm_getpages(prog_size_pages);

    Elf64_Shdr *shdr_base = (Elf64_Shdr *)((uint8_t *)ehdr + ehdr->e_shoff);
    for (uint64_t i = 0; i < ehdr->e_shnum; i++) {
        Elf64_Shdr *shdr = &shdr_base[i];

        if (shdr->sh_addr != 0) {
            //printk("%lx %lx (%lx)\n", shdr->sh_addr, prog->program + (shdr->sh_addr - ELF_VIRT_ENTRY), prog->program);
            //printk("%d\n", shdr->sh_size);
            __paging_map(page_table, (void *)shdr->sh_addr,
                (void *)(prog->program + (shdr->sh_addr - ELF_VIRT_ENTRY)),
                PAGE_NORMAL);

            if (shdr->sh_type == SHT_NOBITS) {
                memset((void *)(prog->program + (shdr->sh_addr - ELF_VIRT_ENTRY)), 0, shdr->sh_size);
            } else {
                memcpy((void *)(prog->program + (shdr->sh_addr - ELF_VIRT_ENTRY)), (uint8_t *)ehdr + shdr->sh_offset, shdr->sh_size);
            }
        }            
    }

    asm volatile ("sti");
    return prog;
}