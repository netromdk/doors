#include "Commands.h"
#include "Lib.h"

void cmdEcho(int argc, char **argv)
{
  for (int i = 1; i < argc; ++i) {
    if (i > 1) {
      putchar(' ');
    }
    print(argv[i]);
  }
  putchar('\n');
}
