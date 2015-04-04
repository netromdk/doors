#include <ctype.h>

bool isblank(int ch) {
  return ch == CTYPE_TAB || ch == CTYPE_SPACE;
}
