# Sequential integration test runner.
# Called by the `check-int-test-all` target.

function(run_qemu_and_check iso_file log_file crash_type)
  message(STATUS "  Running QEMU: ${iso_file}")

  execute_process(
    COMMAND "${QEMU}" -display none
            -machine pc,acpi=on
            -device isa-debug-exit,iobase=0xf4,iosize=0x04
            -no-reboot
            -serial "file:${log_file}"
            -cdrom "${iso_file}" -boot d
    TIMEOUT 30
    RESULT_VARIABLE _qemu_result
  )

  message(STATUS "  QEMU result: ${_qemu_result}")

  if (crash_type)
    set(_check_args --crash-type=${crash_type} --qemu-exit=${_qemu_result})
  else()
    set(_check_args "")
  endif()

  execute_process(
    COMMAND "${PYTHON3}" "${DOORS_SOURCE_DIR}/scripts/parse-test-log.py"
            ${_check_args} "${log_file}"
    RESULT_VARIABLE _check_result
  )

  if (NOT _check_result EQUAL 0)
    message(FATAL_ERROR "FAILED: ${log_file}")
  endif()

  message(STATUS "  PASSED")
endfunction()

message(STATUS "========================================")
message(STATUS "Integration Test Suite (sequential)")
message(STATUS "========================================")

message(STATUS "")
message(STATUS "--- Phase 1/5: Normal integration tests ---")
run_qemu_and_check(
  "${DOORS_BINARY_DIR}/doors-test.iso"
  "${DOORS_BINARY_DIR}/doors-test.log"
  ""
)

message(STATUS "")
message(STATUS "--- Phase 2/5: Crash test (sys_poweroff) ---")
run_qemu_and_check(
  "${DOORS_BINARY_DIR}/doors-crash-poweroff.iso"
  "${DOORS_BINARY_DIR}/doors-crash-poweroff.log"
  "poweroff"
)

message(STATUS "")
message(STATUS "--- Phase 3/5: Crash test (sys_panic) ---")
run_qemu_and_check(
  "${DOORS_BINARY_DIR}/doors-crash-panic.iso"
  "${DOORS_BINARY_DIR}/doors-crash-panic.log"
  "panic"
)

message(STATUS "")
message(STATUS "--- Phase 4/5: Crash test (ioctl_reboot) ---")
run_qemu_and_check(
  "${DOORS_BINARY_DIR}/doors-crash-reboot.iso"
  "${DOORS_BINARY_DIR}/doors-crash-reboot.log"
  "reboot"
)

message(STATUS "")
message(STATUS "--- Phase 5/5: Crash test (ioctl_halt) ---")
run_qemu_and_check(
  "${DOORS_BINARY_DIR}/doors-crash-halt.iso"
  "${DOORS_BINARY_DIR}/doors-crash-halt.log"
  "halt"
)

message(STATUS "")
message(STATUS "========================================")
message(STATUS "All integration tests PASSED")
message(STATUS "========================================")
