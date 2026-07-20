#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <cstring>

int64_t strtoll(const char *str, char **endptr, int base)
{
  const int64_t res = static_cast<int64_t>(strtoull(str, endptr, base));
  if (res == 0) {
    return res;
  }

  // Handle sign.
  const size_t len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    const char ch = str[i];
    if (isspace(ch)) {
      continue;
    }
    else if (ch == '-') {
      return res * -1;
    }
    else {
      break;
    }
  }

  return res;
}
