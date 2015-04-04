#include <ctype.h>

int main() {
  if (tolower('A') != 'a') {
    return 1;
  }

  if (tolower('G') != 'g') {
    return 2;
  }

  if (tolower('a') != 'a') {
    return 3;
  }

  if (tolower('g') != 'g') {
    return 4;
  }
  
  return 0;
}
