#include <kernel/Symbols.h>

extern const SymbolEntry symbol_table[];
extern const int symbol_table_count;

// Do binary search with range matching.
//
// The symbol table is sorted by ascending address (from `nm -n` via "gen-symbols.py"), and each
// function occupies addresses [symbol_table[i].address, symbol_table[i+1].address). From the first
// address to the next but not including. Instead of exact matching, it is enough to find which
// function's range contains the address (the input `addr`).
//
// "gen-symbols.py" appends a sentinel entry with `name=nullptr` and `address=_kernel_end` to bound
// the last real symbol. When the search lands on the sentinel, `nextAddr = 0xFFFFFFFF` and
// `addr < 0xFFFFFFFF` is always true, nullptr is returned.
//
// Example of `symbol_table` entries:
//   ...
//   {0x00100fa9, "(anonymous namespace)::cpuVendorId(char*, unsigned int&)"},
//   {0x0010107f, "(anonymous namespace)::cpuBrandString(char*)"},
//   {0x00101386, "Cpu::init()"},
//   {0x001014c0, "Cpu::dump()"},
//   {0x0010195e, "Cpu::hasVendorId(char const*)"},
//   ...
const char *lookupSymbol(uint32_t addr)
{
  if (!symbol_table_count) {
    return nullptr;
  }

  int lo = 0;
  int hi = symbol_table_count - 1;
  while (lo <= hi) {
    const int mid = lo + (hi - lo) / 2;
    const uint32_t midAddr = symbol_table[mid].address;

    if (addr >= midAddr) {
      const uint32_t nextAddr =
        (mid == symbol_table_count - 1) ? 0xFFFFFFFF : symbol_table[mid + 1].address;
      if (addr < nextAddr) {
        return symbol_table[mid].name;
      }
      lo = mid + 1;
    }
    else {
      hi = mid - 1;
    }
  }

  return nullptr;
}
