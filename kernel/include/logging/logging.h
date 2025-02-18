
#ifndef _KERNEL_LOGGING_H
#define _KERNEL_LOGGING_H

#include <stdint.h>
#include <printk/printk.h>
#include <terminal/terminal.h>

#define LOG_LEVEL_HIGH 0
#define LOG_LEVEL_MEDIUM 1
#define LOG_LEVEL_LOW 2

#define LOG_LEVEL LOG_LEVEL_HIGH

/// @brief Logs out the module name in a green text and the message
/// @param module Name of the module (ex. "APIC", "PCI")
/// @param fmt The text message
void log_ok(char *module, char *fmt, ...);

/// @brief Logs out the module name in a yellow text and the message
/// @param module Name of the module (ex. "APIC", "PCI")
/// @param fmt The text message
void log_warn(char *module, char *fmt, ...);

/// @brief Logs out the module name in a red text and the message
/// @param module Name of the module (ex. "APIC", "PCI")
/// @param fmt The text message
void log_error(char *module, char *fmt, ...);

#endif