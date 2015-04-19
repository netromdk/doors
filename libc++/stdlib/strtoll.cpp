#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

int64_t strtoll(const char *str, char **endptr, int base) {
  int64_t res = strtoull(str, endptr, base);
  if (res == 0) return res;

  // Handle sign.
  size_t len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    char ch = str[i];
    if (isspace(ch)) {
      continue;
    }
    else if (ch == '-') {
      return res * -1;
    }
    else break;
  }

  return res;
}
