#include <cstring>

char *strcpy(char *dst, const char *src)
{
  return static_cast<char *>(memcpy(dst, src, strlen(src) + 1));
}
