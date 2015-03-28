#ifndef STRING_H
#define STRING_H

#include <sys/cdefs.h>
#include <stddef.h>

size_t strlen(const char *str);

void *memcpy(void *dst, const void *src, size_t num);
void *memmove(void *dst, const void *src, size_t num);
void *memset(void *ptr, int value, size_t num);
int memcmp(const void *ptr1, const void *ptr2, size_t num);

#endif // STRING_H
