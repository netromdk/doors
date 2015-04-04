#include <ctype.h>

int main() {
  if (!iscntrl(0)) {
    return 1;
  }

  if (!iscntrl(7)) {
    return 2;
  }

  if (!iscntrl('\t')) {
    return 3;
  }

  if (!iscntrl('\f')) {
    return 4;
  }

  if (!iscntrl('\n')) {
    return 5;
  }

  return 0;
}
