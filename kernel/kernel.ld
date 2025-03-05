OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
	_kern_start = .;
	.text 0x100000 :
	{
		*(.text)
	}
	.data 0x300000 :
	{
		*(.data)
	}
	.rodata 0x500000 :
	{
		*(.rodata)
	}
	.bss 0x700000 :
	{
		*(COMMON)
		*(.bss)
	}
	_kern_end = .;
}