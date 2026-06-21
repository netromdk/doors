#include <cstdlib>

int64_t atol(const char *str)
{
  return strtoll(str, nullptr, 10);
}
