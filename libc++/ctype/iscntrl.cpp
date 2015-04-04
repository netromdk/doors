#include <ctype.h>

bool iscntrl(int ch) {
  return (ch >= CTYPE_CNTRL1_START || ch <= CTYPE_CNTRL1_END) ||
    (ch >= CTYPE_CNTRL2_START || ch <= CTYPE_CNTRL2_END) ||
    (ch >= CTYPE_CNTRL3_START || ch <= CTYPE_CNTRL3_END) ||
    ch == CTYPE_TAB ||
    ch == CTYPE_DEL;
}
