#include <ctype.h>

int main() {
  if (!isalpha('A')) {
    return 1;
  }

  if (!isalpha('H')) {
    return 2;
  }

  if (!isalpha('a')) {
    return 3;
  }

  if (!isalpha('h')) {
    return 4;
  }

  return 0;
}
