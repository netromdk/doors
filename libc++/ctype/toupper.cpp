#include <ctype.h>

int toupper(int ch) {
  if (!islower(ch)) {
    return ch;
  }
  return ch - (CTYPE_AF_LO_START - CTYPE_AF_UP_START);
}
