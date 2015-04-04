#include <ctype.h>

int main() {
  if (!isprint(' ')) {
    return 1;
  }

  if (!isprint('a')) {
    return 2;
  }

  if (!isprint('!')) {
    return 3;
  }

  if (!isprint('=')) {
    return 4;
  }

  if (!isprint('Z')) {
    return 5;
  }

  return 0;
}
