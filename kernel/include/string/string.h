#ifndef _KERNEL_STRING_H
#define _KERNEL_STRING_H

#include <stdint.h>
#include <stddef.h>

void* memset(void* bufptr, int value, size_t size);

void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size);

// Size is NOT in bytes, it is in 32-bit words.
void *memset32(void *bufptr, int value, size_t size);

// Size is NOT in bytes, it is in 64-bit words.
void* memset64(void* bufptr, int value, size_t size);

int memcmp(const void* aptr, const void* bptr, size_t size);

size_t strlen(const char* str);

char *strchr(char *str, int character);

int strcmp(const char *cs, const char *ct);

char *strcpy(char *dest, const char *src);

#endif