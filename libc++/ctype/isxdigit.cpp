#include <ctype.h>

bool isxdigit(int ch) {
  return isdigit(ch) ||
    (ch >= CTYPE_AF_UP_START && ch <= CTYPE_AF_UP_END) ||
    (ch >= CTYPE_AF_LO_START && ch <= CTYPE_AF_LO_END);
}
