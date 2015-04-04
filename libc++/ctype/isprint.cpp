#include <ctype.h>

bool isprint(int ch) {
  return isgraph(ch) || isspace(ch);
}
