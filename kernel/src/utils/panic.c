#include <utils/panic.h>
#include <terminal/terminal.h>
#include <printk/printk.h>

void kernel_panic(const char *message) {
    terminal_set_bg_color_palette(1);
    terminal_clear();
    for (int i = 0; i < 40; i++)
        printk("\xDB");
    printk("\n\xDB");
    printk("\n\xDB  KERNEL PANIC\n");
    printk("\xDB\n");
    for (int i = 0; i < 40; i++)
        printk("\xDB");
    printk("\n\n");

    printk("%s\n", message);
}