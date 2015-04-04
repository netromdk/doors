#include <ctype.h>

int main() {
  for (int i = 0; i < 9; i++) {
    if (!isdigit(i + '1')) {
      return i+1;
    }
  }

  if (isdigit('a')) {
    return 10;
  }

  return 0;
}
