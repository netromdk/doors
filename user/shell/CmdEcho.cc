#include "Commands.h"
#include "Lib.h"

void cmdEcho(const span<string_view> &args)
{
  for (size_t i = 1; i < args.size(); ++i) {
    if (i > 1) {
      putchar(' ');
    }
    print(args[i].data());
  }
  putchar('\n');
}
