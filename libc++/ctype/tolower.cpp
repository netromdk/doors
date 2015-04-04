#include <ctype.h>

int tolower(int ch) {
  if (!isupper(ch)) {
    return ch;
  }
  return ch + (CTYPE_AF_LO_START - CTYPE_AF_UP_START);
}
