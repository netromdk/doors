#include <ctype.h>

bool isalnum(int ch) {
  return isdigit(ch) || isalpha(ch);
}
