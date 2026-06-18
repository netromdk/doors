# Doors

The name is the recursive acronym "Doors of Open Run-time Systems".

## Concept

The challenge was to write a 32-bit OS using C++20 aiming for paged memory, exceptions/interrupts
support, and a keyboard driver. Then dropping into a simple shell for processing simple commands
like querying CPU information, uptime, memory available/used, start/stop/monitor timers etc. Maybe
being able to play a simple "Snake" game or similar.

Things to look into later:

- Floating-point support ([IEEE-754](https://en.wikipedia.org/wiki/IEEE_754-1985))
- Multi-task scheduling
- Real-Time Clock/CMOS support
- Hard disk driver (FAT32 or EXT2, for booting from disk)
- Mouse driver
- USB drivers

## Prerequisites

| Tool | Purpose | Install |
|---|---|---|
| i386-elf cross-compiler | Builds the kernel | `./scripts/bootstrap.sh` |
| CMake 3.25+ | Build system | `sudo apt install cmake` |
| Ninja | Build tool | `sudo apt install ninja-build` |
| Clang or GCC | Host compiler for tests | `sudo apt install clang` |
| QEMU | Run the kernel | `sudo apt install qemu-system-x86` |
| GRUB + mtools | ISO builds | `sudo apt install grub-pc-bin grub-common mtools` |

QEMU and GRUB are optional and only needed for `run`, `run-iso`, and `iso` targets.

### Cross-compiler

The i386-elf cross-compiler (GCC 13.2.0 + Binutils 2.42) must be built before configuring CMake. The
bootstrap script downloads, builds, and installs it into `bootstrap/`:

```sh
./scripts/bootstrap.sh
```

This takes a few minutes. Once built, CMake detects it automatically.

## Build

```sh
# Configure: creates build/default/
cmake --preset default

# Build kernel, libc++, and tests
cd build/default
ninja

# Run tests
ninja test

# Run in QEMU
ninja run

# Build a bootable ISO
ninja iso

# Build ISO and run in QEMU
ninja run-iso
```

### Serial debug preset

Mirrors all `printf` output to `doors.log` via QEMU's COM1 serial port:

```sh
cmake --preset serial-debug
cd build/serial-debug
ninja run   # doors.log is written to build/serial-debug/doors.log
```

### Sanitize preset

Builds tests with `AddressSanitizer` and `UndefinedBehaviorSanitizer`:

```sh
cmake --preset sanitize
cd build/sanitize
ninja test
```

ASan and UBSan are supported by both Clang and GCC. Any violation aborts the test
immediately with a detailed report.

Available presets: `cmake --list-presets`

### Cleaning

```sh
ninja clean   # removes all build artifacts including the test build
```

### Distribution archives

```sh
ninja zip    # build/default/doors.zip
ninja tgz    # build/default/doors.tgz
ninja bz2    # build/default/doors.bz2
ninja xz     # build/default/doors.xz
```

## CMake Options

| Option | Default | Description |
|---|---|---|
| `SERIAL_DEBUG` | `OFF` | Mirror `printf` to COM1 (captured in `doors.log` by QEMU) |
| `BUILD_TESTS` | `ON` | Build tests and include them in the default (`all`) target |
| `VERBOSE_BUILD` | `OFF` | Show raw compiler/linker commands during build |
| `HOST_CXX_COMPILER` | _(auto)_ | Host C++ compiler for tests; auto-detects clang++ then g++ |
| `SANITIZERS` | _(none)_ | Sanitizers for host-compiled tests (e.g. `address;undefined`) |

Pass options at configure time:

```sh
cmake --preset default -DVERBOSE_BUILD=ON
cmake --preset default -DBUILD_TESTS=OFF
cmake --preset default -DHOST_CXX_COMPILER=g++
cmake --preset default -DSANITIZERS="address;undefined"
```

## References

- [Intel i386 manuals](http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html)
- [Logix's i386 reference](http://www.logix.cz/michal/doc/i386/)
- [Modern Operating Systems](http://www.amazon.com/Modern-Operating-Systems-Andrew-Tanenbaum/dp/013359162X/) (book)
- [Operating Systems Principles](http://www.amazon.com/Operating-Systems-Principles-Lubomir-Bic/dp/0130266116) (book)
- [osdev.org](http://wiki.osdev.org/Main_Page)
