#ifndef STRING_H
#define STRING_H

#include <sys/cdefs.h>
#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t num);
void *memmove(void *dst, const void *src, size_t num);
void *memset(void *ptr, int value, size_t num);
int memcmp(const void *ptr1, const void *ptr2, size_t num);
const void *memchr(const void *ptr, int value, size_t num);

size_t strlen(const char *str);
char *strcpy(char *dst, const char *src);

#endif // STRING_H
