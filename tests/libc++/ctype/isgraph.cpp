#include <ctype.h>

int main() {
  if (!isgraph('!')) {
    return 1;
  }

  if (!isgraph('=')) {
    return 2;
  }

  if (!isgraph(']')) {
    return 3;
  }

  if (!isgraph('|')) {
    return 4;
  }

  if (!isgraph('a')) {
    return 5;
  }

  if (isgraph(0)) {
    return 6;
  }

  if (isgraph('\t')) {
    return 7;
  }

  return 0;
}
