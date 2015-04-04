#include <ctype.h>

int main() {
  for (int i = 0; i < 9; i++) {
    if (!isalnum(i + '1')) {
      return i+1;
    }
  }

  if (!isalnum('A')) {
    return 10;
  }

  if (!isalnum('H')) {
    return 11;
  }

  if (!isalnum('a')) {
    return 12;
  }

  if (!isalnum('h')) {
    return 13;
  }

  return 0;
}
