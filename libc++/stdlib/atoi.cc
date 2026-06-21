#include <cstdlib>

int32_t atoi(const char *str)
{
  return strtol(str, nullptr, 10);
}
