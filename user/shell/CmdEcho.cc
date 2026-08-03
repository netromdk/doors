#include "Commands.h"
#include "Lib.h"

int cmdEcho(const span<string_view> &args)
{
  for (size_t i = 1; i < args.size(); ++i) {
    if (i > 1) {
      putchar(' ');
    }
    print(args[i]);
  }
  putchar('\n');
  return EXIT_SUCCESS;
}
