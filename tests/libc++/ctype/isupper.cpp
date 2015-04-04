#include <ctype.h>

int main() {
  if (isupper('a')) {
    return 1;
  }

  if (isupper('g')) {
    return 2;
  }

  if (!isupper('A')) {
    return 3;
  }

  if (!isupper('G')) {
    return 4;
  }

  return 0;
}
