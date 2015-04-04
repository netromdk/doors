#include <ctype.h>

bool isspace(int ch) {
  return ch == CTYPE_SPACE ||
    ch == CTYPE_TAB ||
    (ch >= CTYPE_CNTRL2_START && ch <= CTYPE_CNTRL2_END);
}
