
#ifndef _KERNEL_GOP_H
#define _KERNEL_GOP_H

#include <stdint.h>

typedef struct gop_framebuffer {
	void* address;
	uint64_t buf_size;
	uint32_t width;
	uint32_t height;
	uint32_t pixels_per_sc;
} gop_framebuffer_t;

#endif