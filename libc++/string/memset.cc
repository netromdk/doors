#include <cstdint>
#include <cstring>
#include <cstdio>

void *memset(void *ptr, int value, size_t num)
{
  for (size_t i = 0; i < num; i++) {
    (static_cast<uint8_t *>(ptr))[i] = static_cast<uint8_t>(value);
  }
  return ptr;
}
