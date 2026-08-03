#include "Commands.h"
#include "Lib.h"

int cmdHelp(const span<string_view> &)
{
  print("Commands:\n");
  const auto cmds = getCmdTable();
  for (const auto &cmd : cmds) {
    print("  ");
    print(cmd.name);
    print(" - ");
    print(cmd.desc);
    print("\n");
  }
  return EXIT_SUCCESS;
}
