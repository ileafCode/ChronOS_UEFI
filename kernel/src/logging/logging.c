#include <logging/logging.h>

void log_info(char *module, char *fmt, ...) {
#if LOG_LEVEL == LOG_LEVEL_INFO
    terminal_putc('[');
    terminal_set_fg_color_palette(7);
    terminal_puts(module);
    terminal_set_fg_color_palette(15);
    terminal_puts("]: ");

    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);

    terminal_putc('\n');
#endif
}

void log_ok(char *module, char *fmt, ...)
{
#if LOG_LEVEL <= LOG_LEVEL_HIGH
    terminal_putc('[');
    terminal_set_fg_color_palette(10);
    terminal_puts(module);
    terminal_set_fg_color_palette(15);
    terminal_puts("]: ");

    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);

    terminal_putc('\n');
#endif
}

void log_warn(char *module, char *fmt, ...) {
#if LOG_LEVEL <= LOG_LEVEL_MEDIUM
    terminal_putc('[');
    terminal_set_fg_color_palette(11);
    terminal_puts(module);
    terminal_set_fg_color_palette(15);
    terminal_puts("]: ");

    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);

    terminal_putc('\n');
#endif
}

void log_error(char *module, char *fmt, ...) {
#if LOG_LEVEL <= LOG_LEVEL_LOW
    terminal_putc('[');
    terminal_set_fg_color_palette(9);
    terminal_puts(module);
    terminal_set_fg_color_palette(15);
    terminal_puts("]: ");

    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);

    terminal_putc('\n');
#endif
}
