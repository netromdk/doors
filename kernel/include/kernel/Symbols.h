#ifndef KERNEL_SYMBOLS_H
#define KERNEL_SYMBOLS_H

#include <cstdint>

struct SymbolEntry {
  uint32_t address;
  const char *name;
};

extern const SymbolEntry symbol_table[];
extern const int symbol_table_count;

const char *lookupSymbol(uint32_t addr);

#endif // KERNEL_SYMBOLS_H
