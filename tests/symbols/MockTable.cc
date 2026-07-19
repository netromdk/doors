#include <kernel/Symbols.h>

const SymbolEntry symbol_table[] = {
  {0x100000, "_start"},
  {0x100020, "kmain"},
  {0x100100, "panic"},
  {0x100200, "cmdPanic"},
  {0x100300, "Shell::run"},
  {0x100500, "operator new(unsigned int)"},
  {0x101000, "readCpuState"},
  {0x102000, "dumpBacktrace"},
  {0x102100, nullptr}, // The sentinel.
};

const int symbol_table_count = sizeof(symbol_table) / sizeof(symbol_table[0]);
