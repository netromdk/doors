#include <cstring>

const char *strstr(const char *str, const char *substr)
{
  if (*substr == '\0') {
    return str;
  }
  for (; *str != '\0'; ++str) {
    const char *h = str;
    const char *n = substr;
    while (*h != '\0' && *n != '\0' && *h == *n) {
      ++h;
      ++n;
    }
    if (*n == '\0') {
      return str;
    }
  }
  return nullptr;
}
