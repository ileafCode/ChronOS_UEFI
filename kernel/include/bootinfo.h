
#ifndef _KERNEL_BOOTINFO_H
#define _KERNEL_BOOTINFO_H

#include <stdint.h>
#include <gop.h>
#include <psf1.h>
#include <efimem.h>

typedef struct {
	gop_framebuffer_t *framebuffer;
	psf1_font_t *psf1_Font;
	EFI_MEMORY_DESCRIPTOR *mMap;
	uint64_t mMapSize;
	uint64_t mMapDescSize;
	uint64_t rsdp;
} boot_info_t;

#endif