set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Minimum target ISA level. Instructions above this level are conditionally compiled out or using
# lower level instructions. Valid values: 386, 486, 586, 686.
set(DOORS_TARGET_ISA 586 CACHE STRING "Minimum target ISA level (386, 486, 586, 686)")
add_compile_definitions(DOORS_TARGET_ISA=${DOORS_TARGET_ISA})
message(STATUS "Target ISA: i${DOORS_TARGET_ISA}")

# Build-type-specific flags.
if (DOORS_BUILD_TYPE STREQUAL "Release")
  add_compile_options(
    -O2 -DNDEBUG

    # Keep EBP frame pointer. Doors' Backtrace unwinder walks the saved-EBP chain. Without this flag
    # GCC uses EBP as a general-purpose register and the backtrace becomes unreliable or silent.
    -fno-omit-frame-pointer

    # Signed integer overflow wraps in two's complement instead of being UB. GCC otherwise assumes
    # signed overflow never happens, which can silently remove overflow-dependent logic at -O2.
    -fwrapv

    # Disable type-based alias analysis. Doors dereferences hardware registers through pointers of
    # different types, like VGA RAM, which violates strict aliasing. Without this flag those
    # accesses may be miscompiled at -O2.
    -fno-strict-aliasing
  )
  message(STATUS "Build type: Release (-O2)")
else()
  add_compile_options(-O0 -g)
  message(STATUS "Build type: Debug (-O0)")
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if (VERBOSE_BUILD)
  set(CMAKE_VERBOSE_MAKEFILE ON)
  message(STATUS "Verbose build: ON")
else()
  message(STATUS "Verbose build: OFF")
endif()

if (SERIAL_DEBUG OR KERNEL_UBSAN)
  add_compile_definitions(DEBUG_THROUGH_SERIAL_COM1)
  message(STATUS "Serial debug: ON (printf mirrored to COM1 -> doors.log)")
else()
  message(STATUS "Serial debug: OFF")
endif()

# Host compiler is used for tests and any other host-compiled targets.
if (HOST_CXX_COMPILER)
  find_program(DOORS_HOST_CXX "${HOST_CXX_COMPILER}" NO_CACHE)
  if (NOT DOORS_HOST_CXX)
    message(FATAL_ERROR
      "\n"
      "HOST_CXX_COMPILER is set to '${HOST_CXX_COMPILER}' but it was not found on PATH.\n"
    )
  endif()
else()
  find_program(DOORS_HOST_CXX NAMES clang++ g++ NO_CACHE)
endif()

if (DOORS_HOST_CXX)
  message(STATUS "Host C++ compiler: ${DOORS_HOST_CXX}")
else()
  message(STATUS "Host C++ compiler: not found (tests will be unavailable)")
endif()

if (SANITIZERS)
  message(STATUS "Sanitizers: ${SANITIZERS}")
else()
  message(STATUS "Sanitizers: none")
endif()

if (KERNEL_UBSAN)
  message(STATUS "Kernel UBSan: ON")
else()
  message(STATUS "Kernel UBSan: OFF")
endif()

if (CAPS_LOCK_IS_CTRL)
  add_compile_definitions(CAPS_LOCK_IS_CTRL)
  message(STATUS "Caps Lock remapped as Ctrl: YES")
else()
  message(STATUS "Caps Lock remapped as Ctrl: NO")
endif()

if (BUILD_INTEGRATION_TESTS)
  message(STATUS "Integration tests: ON (testrunner.elf + test ISO/run targets)")
else()
  message(STATUS "Integration tests: OFF")
endif()

include(cmake/analysis.cmake)
