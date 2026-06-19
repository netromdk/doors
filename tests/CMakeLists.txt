cmake_minimum_required(VERSION 3.25)
project(doors_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-O0 -Wall -Wextra)

if (SANITIZERS)
  string(REPLACE ";" "," _san_flag "${SANITIZERS}")
  set(_san_flag "-fsanitize=${_san_flag}")
  add_compile_options(
    "${_san_flag}"
    -fno-omit-frame-pointer   # Preserve frame pointers for accurate ASan stack traces.
    -fno-sanitize-recover=all # Abort on first violation instead of continuing.
  )
  add_link_options("${_san_flag}")
endif()

if(NOT DEFINED DOORS_LIBC_SRC_DIR)
  set(DOORS_LIBC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libc++")
  set(DOORS_LIBC_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libc++/include")
endif()

# doctest vendored single-header at tests/doctest/doctest.h
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
add_library(doctest::doctest ALIAS doctest)

# Host-compiled libc++ for the test binaries.
add_library(libc++test STATIC
  ${DOORS_LIBC_SRC_DIR}/stdio/putchar.cpp
  ${DOORS_LIBC_SRC_DIR}/stdio/puts.cpp
  ${DOORS_LIBC_SRC_DIR}/assert/assertFail.cpp
  ${DOORS_LIBC_SRC_DIR}/stdlib/abort.cpp
  ${DOORS_LIBC_SRC_DIR}/stdlib/itos.cpp
  ${DOORS_LIBC_SRC_DIR}/stdlib/utos.cpp
  ${DOORS_LIBC_SRC_DIR}/stdlib/ltos.cpp
  ${DOORS_LIBC_SRC_DIR}/stdlib/strtol.cpp
  ${DOORS_LIBC_SRC_DIR}/stdlib/strtoul.cpp
  ${DOORS_LIBC_SRC_DIR}/stdlib/strtoll.cpp
  ${DOORS_LIBC_SRC_DIR}/stdlib/strtoull.cpp
  ${DOORS_LIBC_SRC_DIR}/stdlib/atoi.cpp
  ${DOORS_LIBC_SRC_DIR}/stdlib/atol.cpp
  ${DOORS_LIBC_SRC_DIR}/string/memcpy.cpp
  ${DOORS_LIBC_SRC_DIR}/string/memmove.cpp
  ${DOORS_LIBC_SRC_DIR}/string/memset.cpp
  ${DOORS_LIBC_SRC_DIR}/string/memcmp.cpp
  ${DOORS_LIBC_SRC_DIR}/string/memchr.cpp
  ${DOORS_LIBC_SRC_DIR}/string/strlen.cpp
  ${DOORS_LIBC_SRC_DIR}/string/strcpy.cpp
  ${DOORS_LIBC_SRC_DIR}/string/strncpy.cpp
  ${DOORS_LIBC_SRC_DIR}/string/strcmp.cpp
  ${DOORS_LIBC_SRC_DIR}/string/strncmp.cpp
  ${DOORS_LIBC_SRC_DIR}/string/strchr.cpp
  ${DOORS_LIBC_SRC_DIR}/string/strrchr.cpp
)
target_compile_definitions(libc++test PRIVATE __IS_DOORS_LIBCPP __IS_DOORS_LIBCPPTEST)
target_compile_options(libc++test PRIVATE -fno-builtin)
target_include_directories(libc++test PRIVATE ${DOORS_LIBC_INC_DIR})

add_library(test_main STATIC main.cpp)
target_link_libraries(test_main PUBLIC doctest::doctest)

include(CTest)

function(doors_test_module name)
  add_executable(${name} ${ARGN})
  target_include_directories(${name} PRIVATE
    "${DOORS_LIBC_INC_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/include"
  )
  target_link_libraries(${name} PRIVATE test_main libc++test)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

doors_test_module(test_string
  libc++/string/memcpy.cpp
  libc++/string/memset.cpp
  libc++/string/memcmp.cpp
  libc++/string/memmove.cpp
  libc++/string/memchr.cpp
  libc++/string/strlen.cpp
  libc++/string/strcmp.cpp
  libc++/string/strncmp.cpp
  libc++/string/strcpy.cpp
  libc++/string/strncpy.cpp
  libc++/string/strchr.cpp
  libc++/string/strrchr.cpp
)

doors_test_module(test_stdlib
  libc++/stdlib/itos.cpp
  libc++/stdlib/utos.cpp
  libc++/stdlib/ltos.cpp
  libc++/stdlib/strtol.cpp
  libc++/stdlib/strtoul.cpp
  libc++/stdlib/strtoll.cpp
  libc++/stdlib/strtoull.cpp
  libc++/stdlib/atoi.cpp
  libc++/stdlib/atol.cpp
  libc++/stdlib/abs.cpp
  libc++/stdlib/div.cpp
)

doors_test_module(test_stdio
  libc++/stdio/putchar.cpp
  libc++/stdio/puts.cpp
  libc++/stdio/printf.cpp
)

doors_test_module(test_ctype
  libc++/ctype/isalnum.cpp
  libc++/ctype/isalpha.cpp
  libc++/ctype/isblank.cpp
  libc++/ctype/iscntrl.cpp
  libc++/ctype/isdigit.cpp
  libc++/ctype/isxdigit.cpp
  libc++/ctype/isgraph.cpp
  libc++/ctype/ispunct.cpp
  libc++/ctype/isspace.cpp
  libc++/ctype/islower.cpp
  libc++/ctype/isupper.cpp
  libc++/ctype/isprint.cpp
  libc++/ctype/tolower.cpp
  libc++/ctype/toupper.cpp
)

doors_test_module(test_algorithm
  libc++/algorithm/min.cpp
  libc++/algorithm/max.cpp
  libc++/algorithm/swap.cpp
)

# The following tests compile kernel code for host-side testing.

set(HEAP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Heap.cpp")
doors_test_module(test_heap
  heap/kmalloc.cpp
  heap/kfree.cpp
  heap/fragmentation.cpp
  heap/alignment.cpp
  heap/oom.cpp
  "${HEAP_SRC}"
)

set(KBD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/arch/i386/Kbd.cpp")
set(KBD_KEYMAP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Keymap.cpp")
doors_test_module(test_kbd
  kbd/scancode_table.cpp
  kbd/ring_buffer.cpp
  kbd/readline.cpp
  kbd/keymap.cpp
  kbd/modifiers.cpp
  stubs.cpp
  "${KBD_SRC}"
  "${KBD_KEYMAP_SRC}"
)

set(PIT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/arch/i386/Pit.cpp")
doors_test_module(test_pit
  pit/uptime_math.cpp
  stubs.cpp
  "${PIT_SRC}"
)

set(SHELL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Shell.cpp")
set(KBD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/arch/i386/Kbd.cpp")
set(KBD_KEYMAP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Keymap.cpp")
doors_test_module(test_shell
  shell/tokenizer.cpp
  shell/dispatch.cpp
  stubs.cpp
  "${KBD_SRC}"
  "${KBD_KEYMAP_SRC}"
  "${SHELL_SRC}"
)
