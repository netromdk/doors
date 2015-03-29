#ifndef STRING_H
#define STRING_H

#include <sys/cdefs.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memcpy(void *dst, const void *src, size_t num);
void *memmove(void *dst, const void *src, size_t num);
void *memset(void *ptr, int value, size_t num);
int memcmp(const void *ptr1, const void *ptr2, size_t num);
const void *memchr(const void *ptr, int value, size_t num);

size_t strlen(const char *str);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t num);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t num);
const char *strchr(const char *str, int value);
const char *strrchr(const char *str, int value);

#ifdef __cplusplus
}
#endif

#endif // STRING_H
