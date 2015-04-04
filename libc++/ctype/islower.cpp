#include <ctype.h>

bool islower(int ch) {
  return (ch >= CTYPE_AF_LO_START && ch <= CTYPE_AF_LO_END) ||
    (ch >= CTYPE_GZ_LO_START && ch <= CTYPE_GZ_LO_END);
}
