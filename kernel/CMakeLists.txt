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
  ${ARCH_DIR}/Tty.cpp
  ${ARCH_DIR}/Cpu.cpp
  ${ARCH_DIR}/Arch.cpp
  ${ARCH_DIR}/Gdt.cpp
  ${ARCH_DIR}/Idt.cpp
  ${ARCH_DIR}/Io.cpp
  ${ARCH_DIR}/Pic.cpp
  ${ARCH_DIR}/Isr.s
  ${ARCH_DIR}/ExceptionHandlers.cpp
  ${ARCH_DIR}/InterruptHandlers.cpp
  ${ARCH_DIR}/Kbd.cpp
  ${ARCH_DIR}/Pit.cpp
  kernel/Kernel.cpp
  kernel/Runtime.cpp
  kernel/Serial.cpp
  kernel/Mem.cpp
  kernel/Acpi.cpp
  kernel/Cmos.cpp
  kernel/Keymap.cpp
  kernel/Heap.cpp
  kernel/Shell.cpp
)

target_compile_options(kernel_objs PRIVATE
  -ffreestanding -fno-builtin -fno-exceptions -fno-rtti)
target_compile_definitions(kernel_objs PRIVATE __IS_DOORS_KERNEL)
target_include_directories(kernel_objs PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(kernel_objs PRIVATE k)

if (KERNEL_UBSAN)
  target_sources(kernel_objs PRIVATE kernel/Ubsan.cpp)
  target_compile_options(kernel_objs PRIVATE
    -fsanitize=undefined
    -fno-sanitize-recover=all  # Trap immediately. No recovery in a bare-metal kernel.
  )

  # Exclude Ubsan.cpp itself from instrumentation to prevent recursive handler invocations.
  set_source_files_properties(kernel/Ubsan.cpp PROPERTIES
    COMPILE_OPTIONS "-fno-sanitize=all"
  )
endif()

add_executable(doors_kernel $<TARGET_OBJECTS:kernel_objs>)
set_target_properties(doors_kernel PROPERTIES
  OUTPUT_NAME "doors"
  SUFFIX ".kernel"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_link_options(doors_kernel PRIVATE
  -T "${LINKER_SCRIPT}"
  -O0 -ffreestanding -fno-exceptions -fno-rtti -nostdlib
  "${CRTI_OBJ}"
  "${CRTBEGIN_OBJ}")

target_link_libraries(doors_kernel PRIVATE
  "${CRTEND_OBJ}"
  "${CRTN_OBJ}"
  k
  -lgcc)

if (KERNEL_UBSAN)
  target_link_options(doors_kernel PRIVATE -fsanitize=undefined)
endif()

add_dependencies(doors_kernel crt_objects)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/doors.kernel"
  DESTINATION boot
)
