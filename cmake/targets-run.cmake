find_program(QEMU_EXECUTABLE qemu-system-i386)

set(QEMU_FLAGS "")
if (SERIAL_DEBUG)
  list(APPEND QEMU_FLAGS -serial file:${CMAKE_BINARY_DIR}/doors.log)
endif()

add_custom_target(check-qemu
  COMMAND sh -c
    "which qemu-system-i386 > /dev/null 2>&1 || \
     { echo 'Error: qemu-system-i386 not found.'; \
       echo 'Install: sudo apt install qemu-system-x86'; exit 1; }"
  VERBATIM
)

if (QEMU_EXECUTABLE)
  add_custom_target(run
    COMMAND "${QEMU_EXECUTABLE}" ${QEMU_FLAGS} -kernel "${KERNEL_IMG}"
    DEPENDS check-qemu doors_kernel
    VERBATIM
  )
else()
  add_custom_target(run
    COMMAND "${CMAKE_COMMAND}" -E echo
      "Error: qemu-system-i386 not found. Install with: sudo apt install qemu-system-x86"
    COMMAND "${CMAKE_COMMAND}" -E false
    VERBATIM
  )
endif()
