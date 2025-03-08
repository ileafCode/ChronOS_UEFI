#include <terminal/terminal.h>
#include <string/string.h>
#include <io/io.h>
#include <process/process.h>

gop_framebuffer_t *framebuffer;
psf1_font_t *psf1_font;

int cursor_x = 0;
int cursor_y = 0;

int char_height = 0;
uint32_t fg_color = 0;
uint32_t bg_color = 0;

uint32_t palette[] = {
    // Darker colors
    0x000000, // black - 0
    0x7F0000, // red - 1
    0x007F00, // green - 2
    0x7F7F00, // brown - 3
    0x00007F, // blue - 4
    0x7F007F, // magenta - 5
    0x007F7F, // cyan - 6
    0x7F7F7F, // bright gray - 7

    // Brighter colors
    0x3F3F3F, // dark gray - 8
    0xFF0000, // red - 9
    0x00FF00, // green - 10
    0xFFFF00, // yellow - 11
    0x0000FF, // blue - 12
    0xFF00FF, // magenta - 13 
    0x00FFFF, // cyan - 14
    0xFFFFFF, // white - 15
};

void terminal_set_fg_color_palette(uint8_t c) {
    fg_color = palette[c];
}

void terminal_set_bg_color_palette(uint8_t c) {
    bg_color = palette[c];
}

void terminal_clear() {
    cursor_x = 0;
    cursor_y = 0;
    memset32((void *)(framebuffer->address), bg_color, framebuffer->width * framebuffer->height);
}

void terminal_scroll() {
    int row_size = framebuffer->width * char_height; // Width * character height in pixels
    uint32_t *fb = (uint32_t *)framebuffer->address;

    // Move each row up by `char_height` pixels
    for (int y = 0; y < framebuffer->height - char_height; y++) {
        for (int x = 0; x < framebuffer->width; x++) {
            fb[x + y * framebuffer->width] = fb[x + (y + char_height) * framebuffer->width];
        }
    }

    // Clear the last row by setting it to the background color
    memset32(fb + (framebuffer->width * (framebuffer->height - char_height)), 
             bg_color, framebuffer->width * char_height);

    // Move the cursor to the last row
    cursor_y -= char_height;
}

void terminal_putc(unsigned char chr) {
    outb(0xe9, chr);
    return;
    if (chr == '\n') {
        cursor_x = 0;
        cursor_y += char_height;
        if (cursor_y + char_height > framebuffer->height)
            terminal_scroll();
        return;
    }

    unsigned int* pixPtr = (unsigned int*)framebuffer->address;
    char* fontPtr = psf1_font->glyphBuffer + (chr * psf1_font->psf1_Header->charsize);
    for (int y = cursor_y; y < cursor_y + char_height; y++) {
        for (int x = cursor_x; x < cursor_x + TERMINAL_CHAR_WIDTH; x++) {
            if ((*fontPtr & (0b10000000 >> (x - cursor_x))) > 0) {
                *(unsigned int*)(pixPtr + x + (y * framebuffer->pixels_per_sc)) = fg_color;
            } else {
                *(unsigned int*)(pixPtr + x + (y * framebuffer->pixels_per_sc)) = bg_color;
            }
        }
        fontPtr++;
    }

    cursor_x += TERMINAL_CHAR_WIDTH;
    if (cursor_x + TERMINAL_CHAR_WIDTH > framebuffer->width) {
        cursor_x = 0;
        cursor_y += char_height;
    }
    if (cursor_y + char_height > framebuffer->height)
        terminal_scroll();
}

void terminal_puts(char *string) {
    for (int i = 0; string[i] != 0; i++) {
        terminal_putc(string[i]);
    }
}

void terminal_init(gop_framebuffer_t *fb, psf1_font_t *font) {
    framebuffer = fb;
    psf1_font = font;
    char_height = font->psf1_Header->charsize;
    terminal_set_fg_color_palette(15);
    terminal_set_bg_color_palette(0);
}
