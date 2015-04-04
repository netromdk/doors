#include <ctype.h>

int main() {
  if (!ispunct('!')) {
    return 1;
  }

  if (!ispunct('=')) {
    return 2;
  }

  if (!ispunct(']')) {
    return 3;
  }

  if (!ispunct('|')) {
    return 4;
  }

  if (!ispunct('.')) {
    return 5;
  }

  if (!ispunct(',')) {
    return 6;
  }

  return 0;
}
