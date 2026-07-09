function(add_user_program name)
  set(sources)
  set(headers)
  set(definitions "")
  set(next_is_defs FALSE)
  foreach (f ${ARGN})
    if (f STREQUAL "COMPILE_DEFINITIONS")
      set(next_is_defs TRUE)
    elseif (next_is_defs)
      list(APPEND definitions ${f})
      set(next_is_defs FALSE)
    elseif (f MATCHES "\\.h$")
      list(APPEND headers "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
    else()
      list(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
    endif()
  endforeach()

  add_custom_command(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/${name}.elf"
    COMMAND
      ${CMAKE_CXX_COMPILER}
      ${definitions}
      -Wall -Wextra -Wpedantic -Werror
      -ffreestanding -nostdlib -static -no-pie -std=c++20
      -T "${CMAKE_CURRENT_SOURCE_DIR}/../User.ld"
      -I "${CMAKE_CURRENT_SOURCE_DIR}/.."
      -I "${CMAKE_CURRENT_SOURCE_DIR}"
      -I "${CMAKE_SOURCE_DIR}/libc++/include"
      -fno-exceptions -fno-rtti -fno-builtin
      -o "${CMAKE_CURRENT_BINARY_DIR}/${name}.elf"
      ${sources}
      -L "${CMAKE_BINARY_DIR}/libc++"
      -lc++_user
    DEPENDS
      ${sources}
      ${headers}
      "${CMAKE_CURRENT_SOURCE_DIR}/../User.ld"
    VERBATIM
  )

  add_custom_target("${name}" ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${name}.elf")
  add_dependencies("${name}" libcpp_user)

  if(NOT TARGET user_programs)
    add_custom_target(user_programs)
  endif()
  add_dependencies(user_programs "${name}")
endfunction()

function(add_crash_test name module_name)
  set(_ISO "${CMAKE_BINARY_DIR}/doors-crash-${name}.iso")
  set(_TMP "${CMAKE_BINARY_DIR}/tmp_crash_iso_${name}")
  set(_CFG "${CMAKE_BINARY_DIR}/grub-crash-${name}.cfg")

  set(CRASH_NAME "${name}")
  set(CRASH_MODULE "${module_name}")
  configure_file("${CMAKE_SOURCE_DIR}/grub-crash.cfg.in" "${_CFG}" @ONLY)

  add_custom_target(crash-iso-${name}
    COMMAND "${CMAKE_COMMAND}" -E remove_directory "${_TMP}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory   "${_TMP}/boot/grub"
    COMMAND "${CMAKE_COMMAND}" -E copy "${KERNEL_IMG}" "${_TMP}/boot/"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/user/crash-tests/${module_name}.elf"
            "${_TMP}/boot/"
    COMMAND "${CMAKE_COMMAND}" -E copy "${_CFG}" "${_TMP}/boot/grub/grub.cfg"
    COMMAND "${GRUB_MKRESCUE_EXECUTABLE}" -o "${_ISO}" --locale-directory=. "${_TMP}"
    COMMAND "${CMAKE_COMMAND}" -E remove_directory "${_TMP}"
    DEPENDS doors_kernel ${module_name}
    VERBATIM
  )

  if (QEMU_EXECUTABLE)
    add_custom_target(run-int-test-crash-${name}
      COMMAND "${CMAKE_COMMAND}"
              "-DQEMU=${QEMU_EXECUTABLE}"
              "-DSERIAL_LOG=${CMAKE_BINARY_DIR}/doors-crash-${name}.log"
              "-DISO=${_ISO}"
              -P "${CMAKE_SOURCE_DIR}/cmake/run-qemu-timeout.cmake"
      DEPENDS crash-iso-${name}
      VERBATIM
    )
  else()
    add_custom_target(run-int-test-crash-${name}
      COMMAND "${CMAKE_COMMAND}" -E echo "Error: qemu-system-i386 not found."
      COMMAND "${CMAKE_COMMAND}" -E false
      VERBATIM
    )
  endif()

  if (PYTHON3_EXECUTABLE)
    add_custom_target(check-int-test-crash-${name}
      COMMAND "${PYTHON3_EXECUTABLE}"
              "${CMAKE_SOURCE_DIR}/scripts/parse-test-log.py"
              "--crash-type=${name}"
              "${CMAKE_BINARY_DIR}/doors-crash-${name}.log"
      DEPENDS run-int-test-crash-${name}
      VERBATIM
    )
  else()
    add_custom_target(check-int-test-crash-${name}
      COMMAND "${CMAKE_COMMAND}" -E echo "Error: python3 not found."
      COMMAND "${CMAKE_COMMAND}" -E false
      VERBATIM
    )
  endif()
endfunction()
