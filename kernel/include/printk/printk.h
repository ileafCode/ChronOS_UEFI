#ifndef _KERNEL_PRINTK_H
#define _KERNEL_PRINTK_H

#include <stdint.h>
#include <stdarg.h>

void vprintk(const char *fmt, va_list args);
void printk(const char *fmt, ...);

#endif