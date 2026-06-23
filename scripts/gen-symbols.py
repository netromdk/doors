#!/usr/bin/env python3
import os
import re
import subprocess
import sys


def main() -> None:
  if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <kernel-elf> <output-cc>", file=sys.stderr)
    sys.exit(1)

  kernel = sys.argv[1]
  output = sys.argv[2]
  nm_bin = os.environ.get("NM", "nm")
  cxxfilt_bin = os.environ.get("CXXFILT", "c++filt")

  nm_result = subprocess.run(
    [nm_bin, "-n", kernel],
    capture_output=True, text=True, check=True,
  )

  text_symbols = []
  kernel_end = None
  for line in nm_result.stdout.splitlines():
    m = re.match(r"^([0-9a-fA-F]+)\s+([Tt])\s+(.+)$", line)
    if m:
      text_symbols.append((m.group(1), m.group(3)))
    m_end = re.match(r"^([0-9a-fA-F]+)\s+B\s+_kernel_end$", line)
    if m_end:
      kernel_end = m_end.group(1)

  if not text_symbols:
    print("Warning: no text symbols found", file=sys.stderr)

  demangled = []
  if text_symbols:
    cxxfilt_result = subprocess.run(
      [cxxfilt_bin],
      input="\n".join(name for _, name in text_symbols),
      capture_output=True, text=True, check=True,
    )
    demangled = cxxfilt_result.stdout.splitlines()

  with open(output, "w", encoding="utf-8") as f:
    f.write("#include <kernel/Symbols.h>\n\n")
    f.write("const SymbolEntry symbol_table[] = {\n")
    for (addr_hex, _), name in zip(text_symbols, demangled):
      escaped = name.replace('"', '\\"')
      f.write(f"  {{0x{addr_hex.lower()}, \"{escaped}\"}},\n")
    if kernel_end:
      f.write(f"  {{0x{kernel_end.lower()}, nullptr}},\n")
    f.write("};\n\n")
    f.write(
      "const int symbol_table_count = "
      "sizeof(symbol_table) / sizeof(symbol_table[0]);\n"
    )


if __name__ == "__main__":
  main()
