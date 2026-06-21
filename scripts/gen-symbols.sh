#!/bin/bash
set -euo pipefail

if [ $# -ne 2 ]; then
  echo "Usage: $0 <kernel-elf> <output-cc>" >&2
  exit 1
fi

KERNEL="$1"
OUTPUT="$2"

NM="${NM:-nm}"
CXXFILT="${CXXFILT:-c++filt}"

cat > "$OUTPUT" << 'HEADER'
#include <kernel/Symbols.h>

const SymbolEntry symbol_table[] = {
HEADER

"$NM" -n "$KERNEL" \
  | grep -E '^[0-9a-f]+ [Tt] ' \
  | "$CXXFILT" \
  | awk '{
    # Find where the address and type end.
    match($0, /^[0-9a-f]+ [Tt] /)
    name = substr($0, RLENGTH + 1)
    gsub(/"/, "\\\"", name)  # Escape embedded quotes
    printf "  {0x%s, \"%s\"},\n", $1, name
  }' >> "$OUTPUT"

# Append `_kernel_end` as sentinel entry to know when to stop.
KERNEL_END=$("$NM" -n "$KERNEL" | grep -E '^[0-9a-f]+ B _kernel_end' | awk '{print $1}')
if [ -n "$KERNEL_END" ]; then
  echo "  {0x${KERNEL_END}, nullptr}," >> "$OUTPUT"
fi

cat >> "$OUTPUT" << 'FOOTER'
};

const int symbol_table_count = sizeof(symbol_table) / sizeof(symbol_table[0]);
FOOTER
