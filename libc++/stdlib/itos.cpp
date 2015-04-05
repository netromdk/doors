#include <stdlib.h>

void itos(int32_t num, char *str, uint8_t base, bool upcase) {
  if (num < 0) {
    num *= -1;
    str[0] = '-';
    str++;
  }

  ltos(num, str, base, upcase);
}
