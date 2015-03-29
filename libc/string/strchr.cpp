#include <string.h>

/**
 * Searches for the char in the string. Note that \0 is considered
 * part of the string and can also be searched for.
 */
const char *strchr(const char *str, int value) {
  size_t len = strlen(str);
  for (size_t i = 0; i <= len; i++) {
    if (str[i] == (char) value) {
      return str+i;
    }
  }
  return str;
}
