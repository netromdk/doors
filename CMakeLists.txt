cmake_minimum_required(VERSION 3.25)

# Must be set before `project()` so the cross-compiler is active during compiler probing. The
# presets set this via `toolchainFile`. This is a fallback for direct cmake invocations without a
# preset.
if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
    "${CMAKE_SOURCE_DIR}/cmake/i386-elf-toolchain.cmake"
    CACHE FILEPATH "Cross-compilation toolchain")
endif()

project(doors VERSION 0.1 LANGUAGES CXX ASM)

# Generate compile_commands.json for clang-tidy, cppcheck, and coverage.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/options.cmake)
include(cmake/compilation.cmake)

add_subdirectory(libc++)
add_subdirectory(kernel)
add_subdirectory(user)

set(KERNEL_IMG "${CMAKE_BINARY_DIR}/kernel/doors.kernel")

include(cmake/targets-tests.cmake)
include(cmake/targets-run.cmake)
include(cmake/targets-iso.cmake)
include(cmake/targets-dist.cmake)
