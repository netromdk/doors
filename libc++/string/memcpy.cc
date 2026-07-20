#include <cstdint>
#include <cstring>

void *memcpy(void *dst, const void *src, size_t num)
{
  for (size_t i = 0; i < num; i++) {
    (static_cast<uint8_t *>(dst))[i] = (static_cast<const uint8_t *>(src))[i];
  }
  return dst;
}
