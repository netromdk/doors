find_program(GRUB_MKRESCUE_EXECUTABLE grub-mkrescue)
find_program(MFORMAT_EXECUTABLE mformat)
find_program(PYTHON3_EXECUTABLE python3)

set(GRUB_I386_PC_DIR "/usr/lib/grub/i386-pc")

set(ISO_FILE "${CMAKE_BINARY_DIR}/doors.iso")
set(TEST_ISO_FILE "${CMAKE_BINARY_DIR}/doors-test.iso")
set(TMP_ISO "${CMAKE_BINARY_DIR}/tmp_iso")
set(TMP_TEST_ISO "${CMAKE_BINARY_DIR}/tmp_test_iso")

if (BUILD_INTEGRATION_TESTS)
  set(_GRUB_TESTING_ENTRY "menuentry \"doors (testing)\" {
  multiboot /boot/doors.kernel --test
  module /boot/testrunner-interactive.elf
  module /boot/minimal.elf
  module /boot/pagefault-crasher.elf
  module /boot/signal-loop.elf
  module /boot/signal-sigsegv-handler.elf
  module /boot/signal-sigterm-handler.elf
  module /boot/signal-sigkill-handler.elf
}")
else()
  set(_GRUB_TESTING_ENTRY "")
endif()
configure_file("${CMAKE_SOURCE_DIR}/grub.cfg.in"
               "${CMAKE_BINARY_DIR}/grub.cfg" @ONLY)

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
  set(_ISO_COPY_CMDS
    COMMAND "${CMAKE_COMMAND}" -E copy "${KERNEL_IMG}" "${TMP_ISO}/boot/"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/shell/shell.elf"
                                       "${TMP_ISO}/boot/"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/snake/snake.elf"
                                       "${TMP_ISO}/boot/"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/grub.cfg"
                                       "${TMP_ISO}/boot/grub/"
  )
  if (BUILD_INTEGRATION_TESTS)
    list(APPEND _ISO_COPY_CMDS
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/testrunner/testrunner.elf"
                                          "${TMP_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/testrunner/testrunner-interactive.elf"
                                          "${TMP_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/minimal/minimal.elf"
                                          "${TMP_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/pagefault-crasher/pagefault-crasher.elf"
                                          "${TMP_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/signal-tester/signal-loop.elf"
                                          "${TMP_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/signal-tester/signal-sigsegv-handler.elf"
                                          "${TMP_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/signal-tester/signal-sigterm-handler.elf"
                                          "${TMP_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/signal-tester/signal-sigkill-handler.elf"
                                          "${TMP_ISO}/boot/"
    )
  endif()

  add_custom_target(iso
    COMMAND "${CMAKE_COMMAND}" -E remove_directory "${TMP_ISO}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory   "${TMP_ISO}/boot/grub"
    ${_ISO_COPY_CMDS}
    COMMAND "${GRUB_MKRESCUE_EXECUTABLE}" -o "${ISO_FILE}"
            --locale-directory=. "${TMP_ISO}"
    COMMAND "${CMAKE_COMMAND}" -E remove_directory "${TMP_ISO}"
    DEPENDS doors_kernel user_programs
    VERBATIM
  )

  if (BUILD_INTEGRATION_TESTS)
    add_custom_target(test-iso
      COMMAND "${CMAKE_COMMAND}" -E remove_directory "${TMP_TEST_ISO}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory   "${TMP_TEST_ISO}/boot/grub"
      COMMAND "${CMAKE_COMMAND}" -E copy "${KERNEL_IMG}" "${TMP_TEST_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/testrunner/testrunner.elf"
                                         "${TMP_TEST_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/minimal/minimal.elf"
                                          "${TMP_TEST_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/pagefault-crasher/pagefault-crasher.elf"
                                          "${TMP_TEST_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/signal-tester/signal-loop.elf"
                                          "${TMP_TEST_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/signal-tester/signal-sigsegv-handler.elf"
                                          "${TMP_TEST_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/signal-tester/signal-sigterm-handler.elf"
                                          "${TMP_TEST_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/signal-tester/signal-sigkill-handler.elf"
                                          "${TMP_TEST_ISO}/boot/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/grub-test.cfg"
                                         "${TMP_TEST_ISO}/boot/grub/grub.cfg"
      COMMAND "${GRUB_MKRESCUE_EXECUTABLE}" -o "${TEST_ISO_FILE}"
              --locale-directory=. "${TMP_TEST_ISO}"
      COMMAND "${CMAKE_COMMAND}" -E remove_directory "${TMP_TEST_ISO}"
      DEPENDS doors_kernel testrunner minimal pagefault-crasher
              signal-loop signal-sigsegv-handler signal-sigterm-handler signal-sigkill-handler
      VERBATIM
    )

    if (QEMU_EXECUTABLE)
      add_custom_target(run-int-test
        COMMAND "${CMAKE_COMMAND}"
                "-DQEMU=${QEMU_EXECUTABLE}"
                "-DSERIAL_LOG=${CMAKE_BINARY_DIR}/doors-test.log"
                "-DISO=${TEST_ISO_FILE}"
                -P "${CMAKE_SOURCE_DIR}/cmake/run-qemu-timeout.cmake"
        DEPENDS test-iso
        VERBATIM
      )
    else()
      add_custom_target(run-int-test
        COMMAND "${CMAKE_COMMAND}" -E echo
          "Error: qemu-system-i386 not found. Install with: sudo apt install qemu-system-x86"
        COMMAND "${CMAKE_COMMAND}" -E false
        VERBATIM
      )
    endif()

    add_crash_test(panic crash-panic)
    add_crash_test(halt crash-halt)
    add_crash_test(reboot crash-reboot)
    add_crash_test(poweroff crash-poweroff)
  endif()
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

  if (BUILD_INTEGRATION_TESTS)
    add_custom_target(test-iso
      ${_ERR_CMDS}
      VERBATIM
    )
    add_custom_target(run-int-test
      COMMAND "${CMAKE_COMMAND}" -E echo
        "Error: run-int-test requires grub-mkrescue, mformat, and grub i386-pc modules."
      COMMAND "${CMAKE_COMMAND}" -E echo
        "       See 'ninja test-iso' error output for installation instructions."
      COMMAND "${CMAKE_COMMAND}" -E false
      VERBATIM
    )
    foreach(_t panic halt reboot poweroff)
      add_custom_target(crash-iso-${_t} ${_ERR_CMDS} VERBATIM)
      add_custom_target(run-int-test-crash-${_t}
        COMMAND "${CMAKE_COMMAND}" -E echo
          "Error: run-int-test-crash-${_t} requires grub-mkrescue, mformat, and grub i386-pc modules."
        COMMAND "${CMAKE_COMMAND}" -E false
        VERBATIM
      )
    endforeach()
  endif()
endif()

if (QEMU_EXECUTABLE AND GRUB_MKRESCUE_EXECUTABLE)
  add_custom_target(run
    COMMAND "${QEMU_EXECUTABLE}" ${QEMU_FLAGS} -cdrom "${ISO_FILE}" -boot d
    DEPENDS iso
    VERBATIM
  )
else()
  add_custom_target(run
    COMMAND "${CMAKE_COMMAND}" -E echo
      "Error: qemu-system-i386 and/or grub-mkrescue not found"
    COMMAND "${CMAKE_COMMAND}" -E false
    VERBATIM
  )
endif()

if (BUILD_INTEGRATION_TESTS)
  if (PYTHON3_EXECUTABLE)
    add_custom_target(check-int-test
      COMMAND "${PYTHON3_EXECUTABLE}"
              "${CMAKE_SOURCE_DIR}/scripts/parse-test-log.py"
              "${CMAKE_BINARY_DIR}/doors-test.log"
      VERBATIM
    )
  else()
    add_custom_target(check-int-test
      COMMAND "${CMAKE_COMMAND}" -E echo
        "Error: python3 not found. Required for check-int-test"
      COMMAND "${CMAKE_COMMAND}" -E false
      VERBATIM
    )
  endif()

  add_custom_target(check-int-test-all
    COMMAND "${CMAKE_COMMAND}"
            "-DQEMU=${QEMU_EXECUTABLE}"
            "-DPYTHON3=${PYTHON3_EXECUTABLE}"
            "-DDOORS_BINARY_DIR=${CMAKE_BINARY_DIR}"
            "-DDOORS_SOURCE_DIR=${CMAKE_SOURCE_DIR}"
            -P "${CMAKE_SOURCE_DIR}/cmake/run-all-tests.cmake"
    DEPENDS test-iso
            crash-iso-panic crash-iso-halt
            crash-iso-reboot crash-iso-poweroff
    USES_TERMINAL
    VERBATIM
  )
endif()
