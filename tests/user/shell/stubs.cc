#include <span>

#include "Commands.h"

__attribute__((weak)) int cmdHelp(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdClear(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdHalt(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdReboot(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdPanic(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdUptime(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdMemInfo(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdHeap(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdDateTime(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdCpuInfo(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdEcho(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdKill(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdTasks(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdSnake(const span<string_view> &)
{
  return EXIT_SUCCESS;
}

__attribute__((weak)) int cmdStats(const span<string_view> &)
{
  return EXIT_SUCCESS;
}
