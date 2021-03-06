#include <ctype.h>

bool isalpha(int ch) {
  return (ch >= CTYPE_AF_UP_START && ch <= CTYPE_AF_UP_END) ||
    (ch >= CTYPE_GZ_UP_START && ch <= CTYPE_GZ_UP_END) ||
    (ch >= CTYPE_AF_LO_START && ch <= CTYPE_AF_LO_END) ||
    (ch >= CTYPE_GZ_LO_START && ch <= CTYPE_GZ_LO_END);
}
