#include <ctype.h>

bool isdigit(int ch) {
  return (ch >= CTYPE_DIGIT_START && ch <= CTYPE_DIGIT_END);
}
