#ifndef _KERNEL_PRINTK_H
#define _KERNEL_PRINTK_H

#include <stdint.h>
#include <stdarg.h>

void printk(const char *fmt, ...);

#endif