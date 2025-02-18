#ifndef _KERNEL_TERMINAL_H
#define _KERNEL_TERMINAL_H

#include <stdint.h>
#include <gop.h>
#include <psf1.h>

#define TERMINAL_CHAR_WIDTH 8

void terminal_set_fg_color_palette(uint8_t c);
void terminal_set_bg_color_palette(uint8_t c);

void terminal_clear();
void terminal_putc(char chr);
void terminal_puts(char *string);
void terminal_init(gop_framebuffer_t *fb, psf1_font_t *font);

#endif