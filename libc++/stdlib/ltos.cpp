#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>
#include <algorithm.h>

void ltos(uint64_t num, char *str, uint8_t base, bool upcase) {
  size_t i = 0;

  if (num == 0) {
    str[i++] = '0';
  }
  else {
    while (num > 0) {
      int64_t rem = num % base;
      str[i++] = (rem < 10 ? rem + '0' : rem + 'a' - 10);
      if (upcase) {
        str[i-1] = toupper(str[i-1]);
      }
      num = num / base;
    }
  }

  // Terminate string.
  str[i] = 0;

  // Reverse string so "-01" becomes "-10", for instance.
  i--; // Keep zero at the end.
  size_t j = 0;
  while (j < i) {
    swap(str[i], str[j]);
    j++;
    i--;
  }
}
