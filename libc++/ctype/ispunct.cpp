#include <ctype.h>

bool ispunct(int ch) {
  return (ch >= CTYPE_PUNCT1_START && ch <= CTYPE_PUNCT1_END) ||
    (ch >= CTYPE_PUNCT2_START && ch <= CTYPE_PUNCT2_END) ||
    (ch >= CTYPE_PUNCT3_START && ch <= CTYPE_PUNCT3_END) ||
    (ch >= CTYPE_PUNCT4_START && ch <= CTYPE_PUNCT4_END);
}
