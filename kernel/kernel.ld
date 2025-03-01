OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
	.text :
	{
		*(.text)
	}
	.data : 
	{
		*(.data)
	}
	.rodata : 
	{
		*(.rodata)
	}
	.bss : 
	{
		*(COMMON)
		*(.bss)
	}
}