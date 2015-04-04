#include <ctype.h>

int main() {
  if (toupper('a') != 'A') {
    return 1;
  }

  if (toupper('g') != 'G') {
    return 2;
  }

  if (toupper('A') != 'A') {
    return 3;
  }

  if (toupper('G') != 'G') {
    return 4;
  }
  
  return 0;
}
