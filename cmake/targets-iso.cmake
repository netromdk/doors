find_program(GRUB_MKRESCUE_EXECUTABLE grub-mkrescue)
find_program(MFORMAT_EXECUTABLE mformat)

set(GRUB_I386_PC_DIR "/usr/lib/grub/i386-pc")

set(ISO_FILE "${CMAKE_BINARY_DIR}/doors.iso")
set(TMP_ISO  "${CMAKE_BINARY_DIR}/tmp_iso")

set(_ISO_DEPS_OK TRUE)

if (NOT GRUB_MKRESCUE_EXECUTABLE)
  set(_ISO_DEPS_OK FALSE)
endif()
if (NOT MFORMAT_EXECUTABLE)
  set(_ISO_DEPS_OK FALSE)
endif()
if (NOT EXISTS "${GRUB_I386_PC_DIR}")
  set(_ISO_DEPS_OK FALSE)
endif()

if (_ISO_DEPS_OK)
  add_custom_target(iso
    COMMAND "${CMAKE_COMMAND}" -E remove_directory "${TMP_ISO}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory   "${TMP_ISO}/boot/grub"
    COMMAND "${CMAKE_COMMAND}" -E copy "${KERNEL_IMG}" "${TMP_ISO}/boot/"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/grub.cfg"
                                       "${TMP_ISO}/boot/grub/"
    COMMAND "${GRUB_MKRESCUE_EXECUTABLE}" -o "${ISO_FILE}"
            --locale-directory=. "${TMP_ISO}"
    COMMAND "${CMAKE_COMMAND}" -E remove_directory "${TMP_ISO}"
    DEPENDS doors_kernel
    VERBATIM
  )
else()
  set(_ERR_CMDS
    COMMAND "${CMAKE_COMMAND}" -E echo "Error: missing dependencies for ISO target:"
  )
  if (NOT GRUB_MKRESCUE_EXECUTABLE)
    list(APPEND _ERR_CMDS
      COMMAND "${CMAKE_COMMAND}" -E echo "  grub-mkrescue not found."
      COMMAND "${CMAKE_COMMAND}" -E echo "  Install it with: sudo apt install grub-pc-bin grub-common"
    )
  endif()
  if (NOT MFORMAT_EXECUTABLE)
    list(APPEND _ERR_CMDS
      COMMAND "${CMAKE_COMMAND}" -E echo "  mformat not found (required by grub-mkrescue)."
      COMMAND "${CMAKE_COMMAND}" -E echo "  Install it with: sudo apt install mtools"
    )
  endif()
  if (NOT EXISTS "${GRUB_I386_PC_DIR}")
    list(APPEND _ERR_CMDS
      COMMAND "${CMAKE_COMMAND}" -E echo "  GRUB i386-pc BIOS modules not found at ${GRUB_I386_PC_DIR}."
      COMMAND "${CMAKE_COMMAND}" -E echo "  Install them with: sudo apt install grub-pc-bin"
    )
  endif()
  list(APPEND _ERR_CMDS
    COMMAND "${CMAKE_COMMAND}" -E false
  )

  add_custom_target(iso
    ${_ERR_CMDS}
    VERBATIM
  )
endif()

if (QEMU_EXECUTABLE AND GRUB_MKRESCUE_EXECUTABLE)
  add_custom_target(run-iso
    COMMAND "${QEMU_EXECUTABLE}" ${QEMU_FLAGS} -cdrom "${ISO_FILE}" -boot d
    DEPENDS iso
    VERBATIM
  )
else()
  add_custom_target(run-iso
    COMMAND "${CMAKE_COMMAND}" -E echo
      "Error: qemu-system-i386 and/or grub-mkrescue not found"
    COMMAND "${CMAKE_COMMAND}" -E false
    VERBATIM
  )
endif()
