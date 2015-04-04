#include <ctype.h>

int main() {
  if (!isspace(' ')) {
    return 1;
  }

  if (!isspace('\v')) {
    return 2;
  }

  if (!isspace('\f')) {
    return 3;
  }

  if (!isspace('\t')) {
    return 4;
  }

  return 0;
}
