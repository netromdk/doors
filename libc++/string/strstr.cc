#include <cstring>

const char *strstr(const char *str, const char *substr)
{
  if (!*substr) {
    return str;
  }
  for (; *str; ++str) {
    const char *h = str;
    const char *n = substr;
    while (*h && *n && *h == *n) {
      ++h;
      ++n;
    }
    if (!*n) {
      return str;
    }
  }
  return nullptr;
}
