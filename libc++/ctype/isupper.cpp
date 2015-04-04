#include <ctype.h>

bool isupper(int ch) {
  return (ch >= CTYPE_AF_UP_START && ch <= CTYPE_AF_UP_END) ||
    (ch >= CTYPE_GZ_UP_START && ch <= CTYPE_GZ_UP_END);
}
