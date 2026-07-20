option(SERIAL_DEBUG
  "Mirror all printf() output to COM1 (QEMU captures it into doors.log)"
  OFF)

option(BUILD_TESTS
  "Build the test suite and include it in the default (all) target"
  ON)

option(BUILD_INTEGRATION_TESTS
  "Build the integration test runner ELF and enable test ISO/run targets"
  OFF)

option(VERBOSE_BUILD
  "Show raw compiler/linker command lines during build (propagated to sub-builds)"
  OFF)

set(HOST_CXX_COMPILER "" CACHE STRING
  "Host C++ compiler for tests and other host-compiled targets. \
Empty = auto-detect (clang++ preferred, g++ fallback).")

set(SANITIZERS "" CACHE STRING
  "Semicolon-separated sanitizers for host-compiled targets \
(e.g. \"address;undefined\"). Supported: address, undefined.")

option(KERNEL_UBSAN
  "Instrument the kernel with UBSan (freestanding, handlers panic via direct UART I/O)"
  OFF)

option(CAPS_LOCK_IS_CTRL
  "Remap Caps Lock to Left Control (compile-time)"
  OFF)

set(DOORS_BUILD_TYPE "Debug" CACHE STRING
  "Build type: Debug or Release")

option(CLANG_TIDY
  "Run clang-tidy static analysis on host-compiled targets"
  OFF)

option(CPPCHECK
  "Add a cppcheck custom target for static analysis"
  OFF)
