#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

void utos(uint32_t num, char *str, uint8_t base, bool upcase)
{
  size_t i = 0;

  if (num == 0) {
    str[i++] = '0';
  }
  else {
    while (num > 0) {
      const auto rem = num % base;
      str[i++] = static_cast<char>((rem < 10 ? rem + '0' : rem + 'a' - 10));
      if (upcase) {
        str[i - 1] = static_cast<char>(toupper(str[i - 1]));
      }
      num = num / base;
    }
  }

  // Null-terminate then reverse the digit sequence in-place so the most significant digit comes
  // first. Reverse by swapping from both ends toward the middle.
  str[i] = 0;
  i--;
  size_t j = 0;
  while (j < i) {
    const char tmp = str[i];
    str[i] = str[j];
    str[j] = tmp;
    j++;
    i--;
  }
}
