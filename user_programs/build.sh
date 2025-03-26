
SEARCH_DIR="user_programs/lib"
OUTPUT_DIR="user_programs/bin/libc"
LIBRARY_NAME="user_programs/bin/libc.a"

mkdir -p "$OUTPUT_DIR"

echo "Compiling all .c files in $SEARCH_DIR and its subdirectories:"
find "$SEARCH_DIR" -type f -name "*.c" | while read -r file; do
  filename=$(basename "$file" .c)
  x86_64-elf-gcc -I $SEARCH_DIR -c "$file" -o "$OUTPUT_DIR/$filename.o" -O0 -ffreestanding -Wall -Wextra
  echo "Compiled $file to $OUTPUT_DIR/$filename.o"
done

echo "Compiling all .asm files in $SEARCH_DIR and its subdirectories:"
find "$SEARCH_DIR" -type f -name "*.asm" | while read -r file; do
  filename=$(basename "$file" .asm)
  nasm -f elf64 -o "$OUTPUT_DIR/$filename.o" "$file"
  echo "Compiled $file to $OUTPUT_DIR/$filename.o"
done

OBJ_FILES=$(find "$OUTPUT_DIR" -type f -name "*.o")

nasm -f elf64 -o user_programs/bin/test.o user_programs/src/test/test.asm
x86_64-elf-ld -static -Bsymbolic -nostdlib -z max-page-size=0x1000 \
    -T user_programs/link.ld -m elf_x86_64 -o user_programs/bin/test \
    $OBJ_FILES user_programs/bin/test.o

x86_64-elf-gcc -I $SEARCH_DIR -c user_programs/src/init/init.c -o user_programs/bin/init.o -mcmodel=kernel -O0 -ffreestanding
x86_64-elf-ld -static -Bsymbolic -nostdlib -z max-page-size=0x1000 \
    -T user_programs/link.ld -m elf_x86_64 -o user_programs/bin/init \
    $OBJ_FILES user_programs/bin/init.o