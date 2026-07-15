## Doors

A 32-bit x86 hobby OS written in C++20, booted via GRUB/Multiboot.

The name is the recursive acronym "Doors of Open Run-time Systems".

![Doors kernel panic in QEMU](misc/doors_panic.png)

### Features

- Higher-half monolithic kernel loaded at 1 MiB via GRUB/Multiboot
- VGA TTY (80x25)
- PS/2 keyboard (Set 1, shift/ctrl/alt/caps, ring buffer, line editing)
- PIT timer at 1000 Hz with uptime tracking
- Heap allocator (best-fit free-list, coalescing, 16-byte aligned)
- Preemptive Round Robin task scheduler (8 slots, 20 ms quantum)
- `INT 0x80` syscalls (13) for userland/kernel communication
- ELF userland programs loaded as Multiboot modules (Shell with 15 built-in commands, Snake game,
  test runner)
- IDT/PIC with exception handlers and user-mode page fault recovery
- GDT (6 entries, PL0/PL3), TSS for ring transitions
- CMOS/RTC, `CPUID` detection, Multiboot memory map
- Integration test suite with QEMU-based boot testing
- Serial debug (COM1) and kernel UBSan (optional)

For architecture details, see [NOTES.md](NOTES.md).

### Prerequisites

i386-elf cross-compiler (build with `./scripts/bootstrap.sh`),
CMake 3.25+, Ninja, QEMU, GRUB + mtools.

### Build & Run

```
cmake --preset default
cd build/default
ninja
ninja test
ninja run
```

Other presets: `release`, `serial-debug`, `sanitize` (see `cmake --list-presets`).
