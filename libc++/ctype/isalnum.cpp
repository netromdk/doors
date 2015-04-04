#include <ctype.h>

bool isalnum(int ch) {
  return (ch >= CTYPE_DIGIT_START && ch <= CTYPE_DIGIT_END) ||
    isalpha(ch);
}
