OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
	. = 0x10000;
	_kern_start = .;

  	__kernel_code_start = .;
  	.text : AT(ADDR(.text)) {
  	  *(.text)
  	}
  	.rodata ALIGN(4K) : AT(ADDR(.rodata)) {
  	  *(.rodata)
  	}
  	. = ALIGN(4K);

  	.data : AT(ADDR(.data)) {
  	  *(.data)
  	}
  	.bss ALIGN(4K) : AT(ADDR(.bss)) {
  	  *(.bss)
  	}
  	. = ALIGN(4K);
	_kern_end = .;
}