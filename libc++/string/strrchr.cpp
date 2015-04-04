#include <string.h>

/**
 * Searches for the last position of the char in the string. Note that
 * \0 is considered part of the string and can also be searched for.
 */
const char *strrchr(const char *str, int value) {
  size_t len = strlen(str);
  for (size_t i = len; i != 0; i--) {
    if (str[i] == (char) value) {
      return str+i;
    }
  }
  return str;
}
