execute_process(
  COMMAND "${QEMU}" -display none

          # Selects the PIIX4 chipset (default) with ACPI explicitly enabled. Required for
          # SYS_POWEROFF's primary shutdown mechanism: writing 0x3400 to the PIIX4 PMCNTRL register
          # at port 0x604.
          -machine pc,acpi=on

          # Adds QEMU's deterministic exit device. When SYS_POWEROFF writes 1 to port 0x402, QEMU
          # exits immediately with status 0.
          -device isa-debug-exit,iobase=0xf4,iosize=0x04

          # Do not reboot on the triple fault fallback in SYS_POWEROFF. Otherwise it might happen
          # forever.
          -no-reboot

          -serial "file:${SERIAL_LOG}"
          -cdrom "${ISO}" -boot d
  TIMEOUT 30
  RESULT_VARIABLE _result
)

if (_result)
  message(STATUS "run-int-test: QEMU ended with: ${_result}")
endif()
