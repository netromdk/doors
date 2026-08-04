#include "Lib.h"
#include "lib/Syscall.h"

string readLine()
{
  char buf[256];
  const int n = sys_readline(buf, sizeof(buf));
  if (n <= 0) {
    return string();
  }
  return string(buf, static_cast<size_t>(n));
}
