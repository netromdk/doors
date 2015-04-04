#include <ctype.h>

int main() {
  if (!isblank('\t')) {
    return 1;
  }

  if (!isblank(' ')) {
    return 2;
  }

  if (isblank('a')) {
    return 3;
  }

  return 0;
}
