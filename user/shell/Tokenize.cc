#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "Lib.h"

namespace {

bool isDelimiter(char c)
{
  return c == ' ' || c == '\t' || c == '\n';
}

} // namespace

span<string_view> tokenize(const string &line)
{
  // `static` is safe because the shell is single-threaded and the span is consumed before the next
  // call to `tokenize()` overwrites `buf`.
  static array<string_view, 32> buf;

  int argc = 0;
  size_t i = 0;
  while (i < line.size()) {
    while (i < line.size() && isDelimiter(line[i])) {
      ++i;
    }

    if (i >= line.size()) {
      break;
    }
    if (argc >= static_cast<int>(buf.size()) - 1) {
      break;
    }

    const size_t start = i;
    while (i < line.size() && !isDelimiter(line[i])) {
      ++i;
    }

    buf[argc] = string_view(line.data() + start, i - start);
    ++argc;
  }

  buf[argc] = string_view();
  return {buf.data(), static_cast<size_t>(argc)};
}
