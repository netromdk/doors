#include <cstddef>

#include "Util.h"

int brandLen(const char *b)
{
  int n = 0;
  while (n < 48 && b[n]) {
    ++n;
  }
  return n;
}

const char *taskStateStr(const unsigned char st)
{
  switch (st) {
  case 1:
    return "READY";
  case 2:
    return "RUNNING";
  case 3:
    return "BLOCKED";
  default:
    return "DEAD";
  }
}
