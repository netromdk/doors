#ifndef STRING_H
#define STRING_H

#include <cstddef>
#include <sys/cdefs.h>

void *memcpy(void *dst, const void *src, size_t num);
void *memmove(void *dst, const void *src, size_t num);
void *memset(void *ptr, int value, size_t num);
int memcmp(const void *ptr1, const void *ptr2, size_t num);
const void *memchr(const void *ptr, int value, size_t num);

inline constexpr size_t strlen(const char *str)
{
  auto *pos = str;
  for (; *pos; ++pos) {
  }
  return pos - str;
}

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t num);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t num);
const char *strchr(const char *str, int value);
const char *strrchr(const char *str, int value);

#endif // STRING_H
