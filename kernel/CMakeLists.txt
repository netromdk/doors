find_package(Python3 REQUIRED)

set(ARCH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/arch/i386")
set(LINKER_SCRIPT "${ARCH_DIR}/Linker.ld")

set(_crt_built_objs "")
foreach(_crt IN ITEMS Crti Crtn)
  set(_obj "${CMAKE_CURRENT_BINARY_DIR}/${_crt}.o")
  list(APPEND _crt_built_objs "${_obj}")
  add_custom_command(
    OUTPUT  "${_obj}"
    COMMAND ${CMAKE_ASM_COMPILER} -c "${ARCH_DIR}/${_crt}.s" -o "${_obj}" -ffreestanding
    DEPENDS "${ARCH_DIR}/${_crt}.s"
    VERBATIM
  )
endforeach()
list(GET _crt_built_objs 0 CRTI_OBJ)
list(GET _crt_built_objs 1 CRTN_OBJ)
add_custom_target(crt_objects DEPENDS ${_crt_built_objs})

foreach(_crt IN ITEMS crtbegin crtend)
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -ffreestanding -fno-exceptions -fno-rtti
            -print-file-name=${_crt}.o
    OUTPUT_VARIABLE _obj OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (NOT IS_ABSOLUTE "${_obj}" OR NOT EXISTS "${_obj}")
    message(FATAL_ERROR
      "${_crt}.o not found (got '${_obj}'). "
      "The cross-compiler may be incomplete. Try rebuilding with ./scripts/bootstrap.sh."
    )
  endif()
  string(TOUPPER "${_crt}" _upper)
  set(${_upper}_OBJ "${_obj}")
endforeach()

add_library(
  kernel_objs
  OBJECT
  ${ARCH_DIR}/Boot.s
  ${ARCH_DIR}/Tty.cc
  ${ARCH_DIR}/Cpu.cc
  ${ARCH_DIR}/Arch.cc
  ${ARCH_DIR}/Gdt.cc
  ${ARCH_DIR}/Idt.cc
  ${ARCH_DIR}/Io.cc
  ${ARCH_DIR}/Pic.cc
  ${ARCH_DIR}/Isr.s
  ${ARCH_DIR}/ExceptionHandlers.cc
  ${ARCH_DIR}/InterruptHandlers.cc
  ${ARCH_DIR}/Kbd.cc
  ${ARCH_DIR}/Vga.cc
  ${ARCH_DIR}/Pit.cc
  ${ARCH_DIR}/Paging.cc
  kernel/Kernel.cc
  kernel/Pmm.cc
  kernel/Runtime.cc
  kernel/Serial.cc
  kernel/Mem.cc
  kernel/Acpi.cc
  kernel/Cmos.cc
  kernel/Keymap.cc
  kernel/Heap.cc
  kernel/Shell.cc
  kernel/Commands.cc
  kernel/Backtrace.cc
  kernel/Panic.cc
  kernel/Scheduler.cc
  kernel/Semaphore.cc
  kernel/Symbols.cc
  kernel/Taskbar.cc
  programs/api/Screen.cc
  programs/api/Input.cc
  programs/snake/SnakeGame.cc
  programs/snake/Snake.cc
)

target_compile_options(kernel_objs PRIVATE
  -ffreestanding -fno-builtin -fno-exceptions -fno-rtti)
target_compile_definitions(kernel_objs PRIVATE __IS_DOORS_KERNEL)
target_include_directories(kernel_objs PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kernel_objs PRIVATE k)

if (KERNEL_UBSAN)
  target_sources(kernel_objs PRIVATE kernel/Ubsan.cc)
  target_compile_options(kernel_objs PRIVATE
    -fsanitize=undefined
    -fno-sanitize-recover=all  # Trap immediately. No recovery in a bare-metal kernel.
  )
  target_compile_definitions(kernel_objs PRIVATE __IS_DOORS_UBSAN)

  # Exclude Ubsan.cc itself from instrumentation to prevent recursive handler invocations.
  set_source_files_properties(kernel/Ubsan.cc PROPERTIES
    COMPILE_OPTIONS "-fno-sanitize=all"
  )
endif()

# Stub symbol table for first-pass link.
set(STUB_SYMBOL_TABLE "${CMAKE_CURRENT_BINARY_DIR}/stub_symbol_table.cc")
file(WRITE ${STUB_SYMBOL_TABLE}
  "#include <kernel/Symbols.h>\n"
  "const SymbolEntry symbol_table[1] = {};\n"
  "const int symbol_table_count = 0;\n"
)

function(add_kernel_target TARGET_NAME SYMBOL_TABLE_FILE)
  add_executable(${TARGET_NAME}
    $<TARGET_OBJECTS:kernel_objs>
    ${SYMBOL_TABLE_FILE})

  set_target_properties(${TARGET_NAME} PROPERTIES
    SUFFIX ".kernel"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

  target_link_options(${TARGET_NAME} PRIVATE
    -T "${LINKER_SCRIPT}"
    -ffreestanding -fno-exceptions -fno-rtti -nostdlib
    "${CRTI_OBJ}"
    "${CRTBEGIN_OBJ}")

  target_link_libraries(${TARGET_NAME} PRIVATE
    "${CRTEND_OBJ}"
    "${CRTN_OBJ}"
    k
    -lgcc)

  if (KERNEL_UBSAN)
    target_link_options(${TARGET_NAME} PRIVATE -fsanitize=undefined)
  endif()

  add_dependencies(${TARGET_NAME} crt_objects)
endfunction()

# First-pass kernel without symbol table.
add_kernel_target(doors_firstpass "${STUB_SYMBOL_TABLE}")
set_target_properties(doors_firstpass PROPERTIES OUTPUT_NAME "doors_firstpass")

# Generate real symbol table from the first-pass kernel.
set(GENERATED_SYMBOL_TABLE "${CMAKE_CURRENT_BINARY_DIR}/symbol_table.cc")
add_custom_command(
  OUTPUT ${GENERATED_SYMBOL_TABLE}
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/gen-symbols.py"
    "${CMAKE_CURRENT_BINARY_DIR}/doors_firstpass.kernel"
    "${GENERATED_SYMBOL_TABLE}"
  DEPENDS doors_firstpass
  COMMENT "Generating symbol table from kernel.."
  VERBATIM
)

# Final kernel with real symbol table. `.text` precedes `.rodata` in the linker script, so text
# addresses are identical between the first-pass and final link.
add_kernel_target(doors_kernel "${GENERATED_SYMBOL_TABLE}")
set_target_properties(doors_kernel PROPERTIES OUTPUT_NAME "doors")
add_dependencies(doors_kernel doors_firstpass)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/doors.kernel"
  DESTINATION boot
)
