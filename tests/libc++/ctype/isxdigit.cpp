#include <ctype.h>

int main() {
  for (int i = 0; i < 9; i++) {
    if (!isxdigit(i + '1')) {
      return i+1;
    }
  }

  if (!isxdigit('a')) {
    return 10;
  }
  
  if (!isxdigit('e')) {
    return 11;
  }

  if (!isxdigit('F')) {
    return 12;
  }

  return 0;
}
