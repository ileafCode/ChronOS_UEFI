
#ifndef _KERNEL_PSF1_H
#define _KERNEL_PSF1_H

#include <stdint.h>

typedef struct psf1_header {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} psf1_header_t;

typedef struct psf1_font {
	psf1_header_t* psf1_Header;
	void* glyphBuffer;
} psf1_font_t;

#endif
