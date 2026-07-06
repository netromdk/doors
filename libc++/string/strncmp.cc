#include <cstring>

int strncmp(const char *str1, const char *str2, size_t num)
{
  for (size_t i = 0; i < num; ++i) {
    if (str1[i] != str2[i]) {
      return static_cast<unsigned char>(str1[i]) - static_cast<unsigned char>(str2[i]);
    }
    if (str1[i] == '\0') {
      return 0;
    }
  }
  return 0;
}
