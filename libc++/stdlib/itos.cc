#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

void itos(int32_t num, char *str, uint8_t base, bool upcase)
{
  // Handle the sign so the division loop below works on an unsigned value.
  uint32_t unum = static_cast<uint32_t>(num);
  if (num < 0) {
    *str++ = '-';
    unum = -unum;
  }

  size_t i = 0;
  char buf[33]; // 32 binary digits + NUL.

  if (unum == 0) {
    buf[i++] = '0';
  }
  else {
    while (unum > 0) {
      uint32_t rem = unum % base;
      buf[i++] = (rem < 10 ? rem + '0' : rem + 'a' - 10);
      if (upcase) {
        buf[i - 1] = toupper(buf[i - 1]);
      }
      unum = unum / base;
    }
  }

  // Copy the scratch buffer to `str` in reverse order so the most significant digit comes first.
  while (i > 0) {
    *str++ = buf[--i];
  }
  *str = '\0';
}
