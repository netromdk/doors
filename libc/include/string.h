#ifndef STRING_H
#define STRING_H

#include <sys/cdefs.h>
#include <stddef.h>

size_t strlen(const char *str);

void *memcpy(void *dst, const void *src, size_t num);

#endif // STRING_H
