#ifndef USER_COMMANDS_H
#define USER_COMMANDS_H

#include <cstdlib>
#include <span>
#include <string_view>

// Note: `const char*` (not `string`) for `name`/`desc` because `cmdTable` has static
// storage. `string`s non-trivial ctor/dtor require C++ runtime support (`init_array`,
// `__cxa_atexit`) which is unavailable in this freestanding target.
struct Command {
  const char *name;
  const char *desc;
  int (*handler)(const span<string_view> &);
};

// Misuse of a command: missing or invalid arguments.
constexpr int EXIT_USAGE = 2;

// Exit code returned by `dispatch()` for an unrecognized command (POSIX style).
constexpr int EXIT_UNKNOWN_COMMAND = 127;

span<const Command> getCmdTable();

int dispatch(const span<string_view> &);

int cmdHelp(const span<string_view> &);
int cmdClear(const span<string_view> &);
int cmdHalt(const span<string_view> &);
int cmdReboot(const span<string_view> &);
int cmdPanic(const span<string_view> &);
int cmdUptime(const span<string_view> &);
int cmdMemInfo(const span<string_view> &);
int cmdHeap(const span<string_view> &);
int cmdDateTime(const span<string_view> &);
int cmdCpuInfo(const span<string_view> &);
int cmdEcho(const span<string_view> &);
int cmdKill(const span<string_view> &);
int cmdTasks(const span<string_view> &);
int cmdSnake(const span<string_view> &);
int cmdStats(const span<string_view> &);

#endif
