find_program(GRUB_MKRESCUE_EXECUTABLE grub-mkrescue)

set(ISO_FILE "${CMAKE_BINARY_DIR}/doors.iso")
set(TMP_ISO  "${CMAKE_BINARY_DIR}/tmp_iso")

if (GRUB_MKRESCUE_EXECUTABLE)
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
  add_custom_target(iso
    COMMAND "${CMAKE_COMMAND}" -E echo
      "Error: grub-mkrescue not found -- install with: sudo apt install grub-pc-bin grub-common mtools"
    COMMAND "${CMAKE_COMMAND}" -E false
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
