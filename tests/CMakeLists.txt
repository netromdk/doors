cmake_minimum_required(VERSION 3.25)
project(doors_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (DOORS_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-O2 -DNDEBUG -fno-omit-frame-pointer -fwrapv -fno-strict-aliasing)
else()
  add_compile_options(-O0 -g)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

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
  ${DOORS_LIBC_SRC_DIR}/assert/assertFail.cc
  ${DOORS_LIBC_SRC_DIR}/stdio/putchar.cc
  ${DOORS_LIBC_SRC_DIR}/stdio/puts.cc
  ${DOORS_LIBC_SRC_DIR}/stdio/format.cc
  ${DOORS_LIBC_SRC_DIR}/stdlib/abort.cc
  ${DOORS_LIBC_SRC_DIR}/stdlib/atoi.cc
  ${DOORS_LIBC_SRC_DIR}/stdlib/atol.cc
  ${DOORS_LIBC_SRC_DIR}/stdlib/itos.cc
  ${DOORS_LIBC_SRC_DIR}/stdlib/ltos.cc
  ${DOORS_LIBC_SRC_DIR}/stdlib/strtol.cc
  ${DOORS_LIBC_SRC_DIR}/stdlib/strtoll.cc
  ${DOORS_LIBC_SRC_DIR}/stdlib/strtoul.cc
  ${DOORS_LIBC_SRC_DIR}/stdlib/strtoull.cc
  ${DOORS_LIBC_SRC_DIR}/stdlib/utos.cc
  ${DOORS_LIBC_SRC_DIR}/string/memchr.cc
  ${DOORS_LIBC_SRC_DIR}/string/memcmp.cc
  ${DOORS_LIBC_SRC_DIR}/string/memcpy.cc
  ${DOORS_LIBC_SRC_DIR}/string/memmove.cc
  ${DOORS_LIBC_SRC_DIR}/string/memset.cc
  ${DOORS_LIBC_SRC_DIR}/string/strchr.cc
  ${DOORS_LIBC_SRC_DIR}/string/strcmp.cc
  ${DOORS_LIBC_SRC_DIR}/string/strcpy.cc
  ${DOORS_LIBC_SRC_DIR}/string/string.cc
  ${DOORS_LIBC_SRC_DIR}/string/strncmp.cc
  ${DOORS_LIBC_SRC_DIR}/string/strncpy.cc
  ${DOORS_LIBC_SRC_DIR}/string/strrchr.cc
  ${DOORS_LIBC_SRC_DIR}/string/strstr.cc
)
target_compile_definitions(libc++test PRIVATE __IS_DOORS_LIBCPP __IS_DOORS_LIBCPPTEST)
target_compile_options(libc++test PRIVATE -fno-builtin)
target_include_directories(libc++test PRIVATE ${DOORS_LIBC_INC_DIR})
set_target_properties(libc++test PROPERTIES PREFIX "")

add_library(test_main STATIC main.cc)
target_link_libraries(test_main PUBLIC doctest::doctest)

include(CTest)

function(doors_test_module name)
  add_executable(${name} ${ARGN})
  target_include_directories(${name} PRIVATE
    "${DOORS_LIBC_INC_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../kernel"
  )
  target_link_libraries(${name} PRIVATE test_main libc++test)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

doors_test_module(test_libcpp_string_header
  libc++/string/memcpy.cc
  libc++/string/memset.cc
  libc++/string/memcmp.cc
  libc++/string/memmove.cc
  libc++/string/memchr.cc
  libc++/string/strcmp.cc
  libc++/string/strncmp.cc
  libc++/string/strcpy.cc
  libc++/string/strncpy.cc
  libc++/string/strchr.cc
  libc++/string/strrchr.cc
  libc++/string/strstr.cc
)

doors_test_module(test_libcpp_string
  libc++/string/string.cc
)

set(UDIVMODDI3_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../libc++/stdlib/udivmoddi3.cc")

doors_test_module(test_libcpp_stdlib_header
  libc++/stdlib/itos.cc
  libc++/stdlib/utos.cc
  libc++/stdlib/ltos.cc
  libc++/stdlib/strtol.cc
  libc++/stdlib/strtoul.cc
  libc++/stdlib/strtoll.cc
  libc++/stdlib/strtoull.cc
  libc++/stdlib/atoi.cc
  libc++/stdlib/atol.cc
  libc++/stdlib/abs.cc
  libc++/stdlib/div.cc
  libc++/stdlib/udivmoddi3.cc
  "${UDIVMODDI3_SRC}"
)

doors_test_module(test_libcpp_stdio_header
  libc++/stdio/putchar.cc
  libc++/stdio/puts.cc
  libc++/stdio/printf.cc
  libc++/stdio/sprintf.cc
  libc++/stdio/snprintf.cc
)

doors_test_module(test_libcpp_ctype_header
  libc++/ctype/isalnum.cc
  libc++/ctype/isalpha.cc
  libc++/ctype/isblank.cc
  libc++/ctype/iscntrl.cc
  libc++/ctype/isdigit.cc
  libc++/ctype/isxdigit.cc
  libc++/ctype/isgraph.cc
  libc++/ctype/ispunct.cc
  libc++/ctype/isspace.cc
  libc++/ctype/islower.cc
  libc++/ctype/isupper.cc
  libc++/ctype/isprint.cc
  libc++/ctype/tolower.cc
  libc++/ctype/toupper.cc
)

doors_test_module(test_libcpp_algorithm_header
  libc++/algorithm/min.cc
  libc++/algorithm/max.cc
  libc++/algorithm/clamp.cc
  libc++/algorithm/swap.cc
  libc++/algorithm/find.cc
  libc++/algorithm/all_of.cc
  libc++/algorithm/any_of.cc
  libc++/algorithm/none_of.cc
  libc++/algorithm/fill.cc
  libc++/algorithm/count_if.cc
  libc++/algorithm/copy.cc
  libc++/algorithm/swap_ranges.cc
  libc++/algorithm/concepts.cc
)

doors_test_module(test_libcpp_array_header
  libc++/array/array.cc
)

doors_test_module(test_libcpp_string_view
  libc++/string_view/string_view.cc
)

doors_test_module(test_libcpp_numeric_header
  libc++/numeric/accumulate.cc
)

doors_test_module(test_libcpp_utility_header
  libc++/utility/exchange.cc
  libc++/utility/pair.cc
)

doors_test_module(test_libcpp_optional
  libc++/optional/optional.cc
)

doors_test_module(test_libcpp_span
  libc++/span/span.cc
)

# Test that `abort()` actually terminates with a non-zero exit code.
add_executable(test_libcpp_abort libc++/stdlib/abort.cc)
target_include_directories(test_libcpp_abort PRIVATE "${DOORS_LIBC_INC_DIR}")
target_link_libraries(test_libcpp_abort PRIVATE libc++test)
add_test(NAME test_libcpp_abort COMMAND test_libcpp_abort)
set_tests_properties(test_libcpp_abort PROPERTIES WILL_FAIL TRUE)

# The following tests compile kernel code for host-side testing.

set(CMOS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Cmos.cc")
set(HEAP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Heap.cc")
set(PIT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/arch/i386/Pit.cc")
set(KBD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/arch/i386/Kbd.cc")
set(KBD_KEYMAP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Keymap.cc")
set(TTY_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/arch/i386/Tty.cc")
set(SCHED_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Scheduler.cc")
set(SEMAPHORE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Semaphore.cc")
set(PANIC_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Panic.cc")
set(BTRACE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Backtrace.cc")
set(SYMBOLS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Symbols.cc")
set(PMM_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Pmm.cc")
set(SYSCALL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/Syscall.cc")
set(ELF_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../kernel/kernel/ElfLoader.cc")

doors_test_module(test_kernel_heap_initstate
  heap/is_initialized.cc
  stubs.cc
  "${HEAP_SRC}"
)

doors_test_module(test_kernel_heap
  heap/kmalloc.cc
  heap/kfree.cc
  heap/fragmentation.cc
  heap/alignment.cc
  heap/oom.cc
  heap/backwards_coalesce.cc
  stubs.cc
  "${HEAP_SRC}"
)

doors_test_module(test_kernel_kbd
  kbd/scancode_table.cc
  kbd/ring_buffer.cc
  kbd/readline.cc
  kbd/keymap.cc
  kbd/modifiers.cc
  kbd/TryReadKey.cc
  stubs.cc
  "${KBD_SRC}"
  "${KBD_KEYMAP_SRC}"
  "${SEMAPHORE_SRC}"
)

doors_test_module(test_kernel_kbd_caps_is_ctrl
  kbd/caps_is_ctrl.cc
  stubs.cc
  "${KBD_SRC}"
  "${KBD_KEYMAP_SRC}"
  "${SEMAPHORE_SRC}"
)
target_compile_definitions(test_kernel_kbd_caps_is_ctrl PRIVATE CAPS_LOCK_IS_CTRL)

doors_test_module(test_kernel_pit
  pit/uptime_math.cc
  pit/Tickless.cc
  stubs.cc
  "${PIT_SRC}"
)

doors_test_module(test_kernel_tty
  tty/backspace.cc
  tty/cursor.cc
  tty/scrollback.cc
  tty/string_puts.cc
  tty/vga_ram.cc
  tty/lock.cc
  stubs.cc
  "${HEAP_SRC}"
  "${PIT_SRC}"
  "${SCHED_SRC}"
  "${SEMAPHORE_SRC}"
  "${PANIC_SRC}"
  "${BTRACE_SRC}"
  "${SYMBOLS_SRC}"
  "${TTY_SRC}"
)

doors_test_module(test_kernel_symbols
  symbols/test_symbols.cc
  symbols/mock_table.cc
  "${SYMBOLS_SRC}"
)

doors_test_module(test_kernel_panic
  panic/test_panic.cc
  stubs.cc
  "${BTRACE_SRC}"
  "${PANIC_SRC}"
  "${SYMBOLS_SRC}"
)

doors_test_module(test_kernel_scheduler
  scheduler/Init.cc
  scheduler/KillTask.cc
  scheduler/Semaphore.cc
  scheduler/Sleep.cc
  scheduler/StackFrame.cc
  scheduler/RoundRobin.cc
  scheduler/TaskInfo.cc
  scheduler/TaskStates.cc
  scheduler/FpuContext.cc
  scheduler/Tickless.cc
  scheduler/Signals.cc
  stubs.cc
  "${HEAP_SRC}"
  "${PIT_SRC}"
  "${SCHED_SRC}"
  "${SEMAPHORE_SRC}"
  "${PANIC_SRC}"
  "${BTRACE_SRC}"
  "${SYMBOLS_SRC}"
)

doors_test_module(test_kernel_cpu
  cpu/InterruptGuard.cc
  cpu/CpuStub.cc
)

doors_test_module(test_kernel_paging
  paging/ClonePageDir.cc
  paging/PagingStub.cc
  paging/PmmStub.cc
)

doors_test_module(test_kernel_paging_sched
  paging/PhysToVirt.cc
  paging/SchedulerPageDir.cc
  paging/Fork.cc
  paging/Exec.cc
  paging/Waitpid.cc
  paging/ExitCode.cc
  paging/PagingStub.cc
  paging/PmmStub.cc
  stubs.cc
  "${HEAP_SRC}"
  "${PIT_SRC}"
  "${SCHED_SRC}"
  "${SEMAPHORE_SRC}"
  "${PANIC_SRC}"
  "${BTRACE_SRC}"
  "${SYMBOLS_SRC}"
  "${ELF_SRC}"
)
target_compile_definitions(test_kernel_paging_sched PRIVATE __IS_DOORS_KERNEL __DOORS_TESTING)
target_include_directories(test_kernel_paging_sched PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/scheduler"
)

doors_test_module(test_kernel_syscalls
  syscall/SyscallRead.cc
  syscall/SyscallSerial.cc
  syscall/SyscallPoweroff.cc
  tty/vga_ram.cc
  stubs.cc
  "${SYSCALL_SRC}"
  "${HEAP_SRC}"
  "${PIT_SRC}"
  "${KBD_SRC}"
  "${KBD_KEYMAP_SRC}"
  "${TTY_SRC}"
  "${SCHED_SRC}"
  "${SEMAPHORE_SRC}"
  "${PANIC_SRC}"
  "${BTRACE_SRC}"
)

doors_test_module(test_kernel_elf
  elf/ElfLoader.cc
  "${ELF_SRC}"
)

doors_test_module(test_kernel_elf_user
  elf/UserElfTask.cc
  paging/PagingStub.cc
  paging/PmmStub.cc
  stubs.cc
  "${HEAP_SRC}"
  "${PIT_SRC}"
  "${SCHED_SRC}"
  "${SEMAPHORE_SRC}"
  "${PANIC_SRC}"
  "${BTRACE_SRC}"
  "${SYMBOLS_SRC}"
  "${ELF_SRC}"
)
target_compile_definitions(test_kernel_elf_user PRIVATE __IS_DOORS_KERNEL __DOORS_TESTING)

doors_test_module(test_kernel_cmos
  kernel/Cmos.cc
)

set(SHELL_COMMANDS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../user/shell/Commands.cc")
set(SHELL_TOKENIZE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../user/shell/Tokenize.cc")
set(SHELL_UTIL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../user/shell/Util.cc")
doors_test_module(test_user_shell
  user/shell/Dispatch.cc
  user/shell/Tokenize.cc
  user/shell/BrandLen.cc
  user/shell/StateStr.cc
  user/shell/stubs.cc
  "${SHELL_COMMANDS_SRC}"
  "${SHELL_TOKENIZE_SRC}"
  "${SHELL_UTIL_SRC}"
)
target_include_directories(test_user_shell PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/user/shell"
  "${CMAKE_CURRENT_SOURCE_DIR}/../user/shell"
)

set(SNAKE_GAME_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../user/snake/Game.cc")
doors_test_module(test_user_snake
  user/snake/Direction.cc
  user/snake/Body.cc
  user/snake/BoostZones.cc
  user/snake/Collision.cc
  user/snake/Food.cc
  user/snake/Prng.cc
  user/snake/BonusFood.cc
  user/snake/Obstacles.cc
  user/snake/stubs.cc
  "${SNAKE_GAME_SRC}"
)
target_include_directories(test_user_snake PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../user/snake"
)
