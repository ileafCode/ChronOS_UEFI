
#include <string/string.h>

void* memset(void* bufptr, int value, size_t size) {
	uint8_t* buf = (uint8_t*) bufptr;
	for (size_t i = 0; i < size; i++)
		buf[i] = (uint8_t) value;
	return bufptr;
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