#include <cstdlib>

int32_t strtol(const char *str, char **endptr, int base)
{
  return static_cast<int32_t>(strtoll(str, endptr, base));
}
