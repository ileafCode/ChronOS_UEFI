#include <printk/printk.h>
#include <string/string.h>
#include <terminal/terminal.h>

#define PRINTK_FLAG_LONG 1 << 0
#define PRINTK_FLAG_FIELD_WIDTH 1 << 1

#include <stdbool.h>

char *itoa(int64_t value, char *buffer, int base) {
    // Check for valid base
    if (base < 2 || base > 36) {
        buffer[0] = '\0';
        return buffer;
    }

    char *ptr = buffer;
    bool isNegative = false;

    // For base 10, record negative sign.
    if (value < 0 && base == 10) {
        isNegative = true;
    }

    // Use unsigned long long to safely handle INT_MIN.
    uint64_t uvalue;
    if (value < 0)
        uvalue = -(uint64_t)value;
    else
        uvalue = value;

    // Convert the number to the given base in reverse order.
    do {
        int64_t remainder = uvalue % base;
        *ptr++ = (remainder < 10) ? ('0' + remainder)
                                  : ('A' + remainder - 10);
        uvalue /= base;
    } while (uvalue != 0);

    // Append negative sign if needed.
    if (isNegative)
        *ptr++ = '-';

    *ptr = '\0';

    // Reverse the string in-place.
    char *start = buffer;
    char *end = ptr - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
    return buffer;
}

int isdigit(char c) {
    if ((c >= '0') && (c <= '9')) return 1;
    return 0;
}

void vprintk(const char *fmt, va_list args) {
    uint8_t is_format = 0;
	uint16_t flags = 0;

    for (; *fmt; fmt++) {
        if (*fmt != '%' && !is_format) {
			terminal_putc(*fmt);
			continue;
		}

        if (!is_format) {
		    is_format = 1;
            flags = 0;
            continue;
        }

        int field_width = 0;

        while (isdigit(*fmt)) {
            field_width = field_width * 10 + (*fmt - '0');
            fmt++;
        }

        if (*fmt == 'l') {
            flags |= PRINTK_FLAG_LONG;
            fmt++;
        }

        switch (*fmt) {
        case 'd': {
            int64_t num;
			if (flags & PRINTK_FLAG_LONG)
				num = (int64_t) va_arg(args, int64_t);
			else 
				num = (int64_t) va_arg(args, int);
            char temp[32];
            itoa(num, temp, 10);
            terminal_puts(temp);
			is_format = 0;
            break;
        }
		case 'x': {
            int64_t num;
			if (flags & PRINTK_FLAG_LONG)
				num = (int64_t) va_arg(args, int64_t);
			else 
				num = (int64_t) va_arg(args, int);
            char temp[32];
            itoa(num, temp, 16);

            if (field_width > strlen(temp)) {
                for (int i = 0; i < field_width - strlen(temp); i++) {
                    terminal_putc(' ');
                }
            }

            terminal_puts(temp);
			is_format = 0;
            break;
        }
		case 's': {
            char *str = (char *) va_arg(args, char *);
            terminal_puts(str);
			is_format = 0;
            break;
		}
        }
    }
}

void printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}