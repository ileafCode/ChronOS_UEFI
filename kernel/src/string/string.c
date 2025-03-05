
#include <string/string.h>

void* memset(void* bufptr, int value, size_t size) {
	uint8_t* buf = (uint8_t*) bufptr;
	for (size_t i = 0; i < size; i++)
		buf[i] = (uint8_t) value;
	return bufptr;
}

void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];
	return dstptr;
}

void *memset32(void *bufptr, int value, size_t size) {
	uint32_t* buf = (uint32_t*) bufptr;
	for (size_t i = 0; i < size; i++)
		buf[i] = (uint32_t) value;
	return bufptr;
}

void *memset64(void *bufptr, int value, size_t size) {
	uint64_t* buf = (uint64_t*) bufptr;
	for (size_t i = 0; i < size; i++)
		buf[i] = (uint64_t) value;
	return bufptr;
}

int memcmp(const void* aptr, const void* bptr, size_t size) {
	const unsigned char* a = (const unsigned char*) aptr;
	const unsigned char* b = (const unsigned char*) bptr;
	for (size_t i = 0; i < size; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}


char *strchr(char *str, int character) {
	do {
		if (*str == character) {
			return (char *)str;
		}
	} while (*str++);
	return 0;
}

int strcmp(const char *cs, const char *ct) {
	unsigned char c1, c2;
	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

char *strcpy(char *dest, const char *src) {
	char *tmp = dest;
	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}