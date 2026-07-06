#include <cstddef>

#include "Lib.h"
#include "lib/Syscall.h"

void print(const char *s)
{
  unsigned int len = 0;
  while (s[len]) {
    ++len;
  }
  sys_write_str(s, len);
}

string readLine()
{
  char buf[256];
  const int n = sys_readline(buf, sizeof(buf));
  if (n <= 0) {
    return string();
  }
  return string(buf, static_cast<size_t>(n));
}
