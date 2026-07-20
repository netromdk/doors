#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdint>

uint64_t strtoull(const char *str, char **endptr, int base)
{
  uint64_t res = 0;

  const auto len = strlen(str);
  size_t i = 0;
  for (; i < len; i++) {
    char ch = str[i];

    // Ignore signs here, it will be handle by strtoll() instead.
    if (ch == '-' || ch == '+') { // NOLINT(bugprone-branch-clone)
      continue;
    }

    // Ignore whitespace until first found digit.
    else if (isspace(ch) && res == 0) {
      continue;
    }

    else if (base == 0 && res == 0) {
      // Detect base 16 (hexadecimal) if number is preceded by a "0x" or "0X".
      if (i + 1 != len && ch == '0' && (str[i + 1] == 'x' || str[i + 1] == 'X')) {
        base = 16;
        i++;
        continue;
      }

      // Detect base 8 (octal) if number is preceded by a zero.
      else if (ch == '0') {
        base = 8;
        continue;
      }
    }

    if (!(base != 16 ? isdigit(ch) : isxdigit(ch))) {
      break;
    }

    // If base could not be auto-detected then use decimal 10.
    if (base == 0) {
      base = 10;
    }

    if (isupper(ch)) {
      ch = static_cast<char>(tolower(ch));
    }

    int ich = ch - '0';
    if (ich > 10) {
      ich = ich - ('0' - 10) - 1;
    }

    res = (res * base) + ich;
  }

  if (endptr != nullptr) {
    *endptr = const_cast<char *>(str) + i;
  }

  return res;
}
