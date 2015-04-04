#include <ctype.h>

bool isgraph(int ch) {
  return ispunct(ch) || isalnum(ch);
}
