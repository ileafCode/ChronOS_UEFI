nasm -f elf64 -o user_programs/bin/test.o user_programs/test.asm
x86_64-elf-ld -T user_programs/link.ld -m elf_x86_64 -o user_programs/bin/test user_programs/bin/test.o