#include <stdlib.h>

void itos(int32_t num, char *str, uint8_t base) {
  if (num < 0) {
    num *= -1;
    str[0] = '-';
    str++;
  }

  utos(num, str, base);
}
