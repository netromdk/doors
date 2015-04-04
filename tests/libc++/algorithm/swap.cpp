#include <algorithm.h>

int main() {
  int a = 1, b = 2;
  swap(a, b);
  if (a != 2 || b != 1) {
    return 1;
  }
  
  return 0;
}
