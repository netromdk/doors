#include <ctype.h>

int main() {
  if (!islower('a')) {
    return 1;
  }

  if (!islower('g')) {
    return 2;
  }

  if (islower('A')) {
    return 3;
  }

  if (islower('G')) {
    return 4;
  }

  return 0;
}
