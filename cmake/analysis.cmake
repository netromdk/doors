if (CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy)
  if (CLANG_TIDY_EXE)
    message(STATUS "clang-tidy: ${CLANG_TIDY_EXE}")
    set(_DOORS_CLANG_TIDY_EXTRA "")
    if (DOORS_HOST_CXX)
      # Clang-tidy is Clang, so `DOCTEST_CLANG` is defined in "doctest.h".
      execute_process(COMMAND ${DOORS_HOST_CXX} --version
        OUTPUT_VARIABLE _host_cxx_version ERROR_QUIET)

      # GCC: -stdlib=libstdc++ finds `<version>`, avoiding the "doctest.h" `<ciso646>` fallback
      # path.
      # Clang: defaults to libc++ which has all headers. No extra args needed.
      if (NOT _host_cxx_version MATCHES "clang")
        execute_process(COMMAND ${DOORS_HOST_CXX} -dumpversion
          OUTPUT_VARIABLE _gcc_version OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${DOORS_HOST_CXX} -dumpmachine
          OUTPUT_VARIABLE _gcc_machine OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(_gcc_install_dir "/usr/lib/gcc/${_gcc_machine}/${_gcc_version}")
        set(_DOORS_CLANG_TIDY_EXTRA
          "--extra-arg=-stdlib=libstdc++;--extra-arg=--gcc-install-dir=${_gcc_install_dir}")
      endif()
    endif()
    if (_DOORS_CLANG_TIDY_EXTRA)
      set(_DOORS_CLANG_TIDY_EXE "${CLANG_TIDY_EXE};${_DOORS_CLANG_TIDY_EXTRA}")
    else()
      set(_DOORS_CLANG_TIDY_EXE "${CLANG_TIDY_EXE}")
    endif()
  else()
    message(WARNING "CLANG_TIDY=ON but clang-tidy was not found on PATH")
  endif()
else()
  message(STATUS "clang-tidy: OFF")
endif()

if (CPPCHECK)
  find_program(CPPCHECK_EXE NAMES cppcheck)
  if (CPPCHECK_EXE)
    message(STATUS "cppcheck: ${CPPCHECK_EXE}")
  else()
    message(WARNING "CPPCHECK=ON but cppcheck was not found on PATH")
  endif()
else()
  message(STATUS "cppcheck: OFF")
endif()

if (CODE_COVERAGE)
  find_program(LCOV_EXE lcov)
  find_program(GENHTML_EXE genhtml)
  if (NOT LCOV_EXE OR NOT GENHTML_EXE)
    set(_missing "")
    if (NOT LCOV_EXE)
      string(APPEND _missing "  lcov\n")
    endif()
    if (NOT GENHTML_EXE)
      string(APPEND _missing "  genhtml\n")
    endif()
    message(FATAL_ERROR
      "CODE_COVERAGE is ON but the following tools were not found on PATH:\n"
      "${_missing}"
      "Install them:\n"
      "  sudo apt install lcov\n"
    )
  endif()
  message(STATUS "Code coverage: ON (-fprofile-arcs -ftest-coverage)")
else()
  message(STATUS "Code coverage: OFF")
endif()
