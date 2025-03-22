#ifndef _KERNEL_DEV_FB_H
#define _KERNEL_DEV_FB_H

#include <stdint.h>
#include <stddef.h>
#include <gop.h>

#define FB_IOCTL_DATA 1

typedef struct fb_ioctl_data {
    uint16_t width, height;
    int fb_size;
} __attribute__((packed)) fb_ioctl_data_t;

int fb_init(gop_framebuffer_t *fb);

#endif