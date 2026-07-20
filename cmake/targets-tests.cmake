include(ExternalProject)

if (BUILD_TESTS AND NOT DOORS_HOST_CXX)
  message(FATAL_ERROR
    "\n"
    "BUILD_TESTS is ON but no host C++ compiler was found on PATH.\n"
    "\n"
    "Install one:\n"
    "  sudo apt install clang\n"
    "  sudo apt install g++\n"
    "\n"
    "Or pin a specific compiler:\n"
    "  cmake -DHOST_CXX_COMPILER=clang++ ...\n"
    "\n"
    "Or disable the test suite:\n"
    "  cmake -DBUILD_TESTS=OFF ...\n"
  )
endif()

set(_ep_exclude "")
if (NOT BUILD_TESTS)
  set(_ep_exclude EXCLUDE_FROM_ALL TRUE)
endif()

if (VERBOSE_BUILD)
  set(_build_flags "-v")
else()
  set(_build_flags "--quiet")
endif()

ExternalProject_Add(tests
  PREFIX     "${CMAKE_BINARY_DIR}/tests"
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/tests"
  BINARY_DIR "${CMAKE_BINARY_DIR}/tests"
  STAMP_DIR  "${CMAKE_BINARY_DIR}/tests/stamps"
  CMAKE_ARGS
    "-G${CMAKE_GENERATOR}"
    "-DCMAKE_CXX_COMPILER=${DOORS_HOST_CXX}"
    "-DCMAKE_AR=ar"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    "-DDOORS_LIBC_SRC_DIR=${CMAKE_SOURCE_DIR}/libc++"
    "-DDOORS_LIBC_INC_DIR=${CMAKE_SOURCE_DIR}/libc++/include"
    "-DDOORS_BUILD_TYPE=${DOORS_BUILD_TYPE}"
  CMAKE_CACHE_ARGS
    "-DSANITIZERS:STRING=${SANITIZERS}"
    "-DCODE_COVERAGE:BOOL=${CODE_COVERAGE}"
    "-DCMAKE_CXX_CLANG_TIDY:STRING=${_DOORS_CLANG_TIDY_EXE}"
  BUILD_COMMAND   "${CMAKE_MAKE_PROGRAM}" -C <BINARY_DIR> ${_build_flags}
  BUILD_ALWAYS    TRUE
  INSTALL_COMMAND ""
  TEST_COMMAND    ""
  ${_ep_exclude}
)

set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES
  "${CMAKE_BINARY_DIR}/tests"
)

add_custom_target(test
  COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}/tests"
          --output-on-failure
  DEPENDS tests
  VERBATIM
)

if (CODE_COVERAGE)
  # Derive the gcov binary that matches the host compiler.
  #
  # clang++: generate a wrapper that calls `llvm-cov gcov` (clang's gcov emulation mode).
  #          lcov cannot use clang++ directly as a gcov tool.
  #
  # g++:     derive matching gcov-N from the compiler path, gcov-14 for g++-14.
  #          Using amismatched gcov causes lcov to abort with a version error.
  set(_DOORS_GCOV_EXE "")
  if (DOORS_HOST_CXX MATCHES "clang")
    set(_llvm_cov_names llvm-cov)
    foreach(_ver RANGE 30 11 -1)
      list(APPEND _llvm_cov_names "llvm-cov-${_ver}")
    endforeach()
    find_program(_llvm_cov NAMES ${_llvm_cov_names} NO_CACHE)
    if (_llvm_cov)
      set(_gcov_wrapper "${CMAKE_BINARY_DIR}/llvm-cov-gcov.sh")
      file(WRITE "${_gcov_wrapper}"
        "#!/bin/sh\nexec ${_llvm_cov} gcov \"$@\"\n")
      file(CHMOD "${_gcov_wrapper}"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
      set(_DOORS_GCOV_EXE "${_gcov_wrapper}")
    endif()
  elseif (DOORS_HOST_CXX MATCHES "g\\+\\+")
    get_filename_component(_host_dir "${DOORS_HOST_CXX}" DIRECTORY)
    get_filename_component(_host_name "${DOORS_HOST_CXX}" NAME)
    if (_host_name MATCHES "^g\\+\\+")
      string(REGEX REPLACE "^g\\+\\+" "gcov" _gcov_name "${_host_name}")
      set(_gcov_candidate "${_host_dir}/${_gcov_name}")
      if (EXISTS "${_gcov_candidate}")
        set(_DOORS_GCOV_EXE "${_gcov_candidate}")
      else()
        set(_gcov_candidate "${_host_dir}/gcov")
        if (EXISTS "${_gcov_candidate}")
          set(_DOORS_GCOV_EXE "${_gcov_candidate}")
        endif()
      endif()
    endif()
  endif()

  if (_DOORS_GCOV_EXE)
    message(STATUS "gcov tool: ${_DOORS_GCOV_EXE}")
  else()
    message(FATAL_ERROR
      "CODE_COVERAGE is ON but a matching gcov tool could not be found for "
      "host compiler '${DOORS_HOST_CXX}'.\n"
      "\n"
      "For g++: install the matching gcov (e.g. sudo apt install gcc-14).\n"
      "For clang++: install the LLVM tools (e.g. sudo apt install llvm).\n"
    )
  endif()

  set(_lcov_args --quiet)
  if (_DOORS_GCOV_EXE)
    list(APPEND _lcov_args --gcov-tool "${_DOORS_GCOV_EXE}")
  endif()

  set(_cov_dir "${CMAKE_BINARY_DIR}/coverage")
  set(_cov_info "${_cov_dir}/coverage.info")
  set(_cov_html "${_cov_dir}/html")
  set(_cov_report "${_cov_dir}/report.txt")
  set(_cov_port "8080")
  set(_cov_url "http://localhost:${_cov_port}")

  add_custom_target(coverage
    COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${_cov_dir}"

    # Reset all execution counters to zero before running tests.
    COMMAND ${LCOV_EXE} --zerocounters
            --directory "${CMAKE_BINARY_DIR}/tests"
            --quiet

    # Run the full test suite with failures are printed to stderr.
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}/tests"
            --output-on-failure

    # Capture coverage data from the test binaries. `--no-external` excludes system/third-party
    # headers.
    COMMAND ${LCOV_EXE} --capture
            --directory "${CMAKE_BINARY_DIR}/tests"
            --base-directory "${CMAKE_SOURCE_DIR}"
            --output-file "${_cov_info}"
            --no-external
            --ignore-errors gcov,inconsistent,unsupported,empty,format,count
            ${_lcov_args}

    # Strip out the Doctest header-only library from the report.
    COMMAND ${LCOV_EXE} --remove
            "${_cov_info}"
            */tests/doctest/*
            --output-file "${_cov_info}"
            --ignore-errors inconsistent,unsupported,format,empty,count,unused
            --quiet

    # Generate an HTML report from the filtered .info file.
    COMMAND ${GENHTML_EXE}
            "${_cov_info}"
            --output-directory "${_cov_html}"
            --ignore-errors corrupt,inconsistent,count,empty,category
            --quiet

    COMMAND "${CMAKE_COMMAND}" -E echo
            "Coverage report: ${_cov_html}/index.html"
    DEPENDS tests
    COMMENT "Generating lcov coverage report..."
    VERBATIM
  )

  find_program(PYTHON3_EXE NAMES python3 python)
  if (PYTHON3_EXE)
    add_custom_target(coverage-serve
      COMMAND sh -c
        "${PYTHON3_EXE} -m http.server ${_cov_port} --directory ${_cov_html} & \
         BGPID=\$!; sleep 1; xdg-open ${_cov_url}; wait \$BGPID"
      DEPENDS coverage
      WORKING_DIRECTORY "${_cov_html}"
      COMMENT "Serving coverage report at ${_cov_url}"
      VERBATIM
    )
  endif()

  add_custom_target(coverage-report
    COMMAND sh -c
      "(${LCOV_EXE} --list ${_cov_info} \
         --ignore-errors empty; \
       echo; \
       ${LCOV_EXE} --summary ${_cov_info} \
         --ignore-errors empty) \
      | tee ${_cov_report}"
    DEPENDS coverage
    COMMENT "Coverage report (also saved to coverage/report.txt):"
    VERBATIM
  )
endif()

if (CPPCHECK AND CPPCHECK_EXE)
  find_program(JQ_EXE NAMES jq)
  if (JQ_EXE)
    add_custom_target(cppcheck
      COMMAND "${CMAKE_COMMAND}"
        -DJQ_EXE=${JQ_EXE}
        -DINPUT_FILE=${CMAKE_BINARY_DIR}/tests/compile_commands.json
        -DOUTPUT_FILE=${CMAKE_BINARY_DIR}/compile_commands_cppcheck.json
        -DEXCLUDE_REGEX="tests/doctest|concepts.cc"
        -P ${CMAKE_SOURCE_DIR}/cmake/filter-compile-commands.cmake
      COMMAND ${CPPCHECK_EXE}
        --project=${CMAKE_BINARY_DIR}/compile_commands_cppcheck.json
        --enable=warning,style,performance,portability
        --suppress=missingInclude
        --suppress=unknownMacro
        --suppress=preprocessorErrorDirective
        --suppress=staticStringCompare
        --suppress=cstyleCast
        --suppress=useStandardLibrary
        --suppress=uninitMemberVar
        --suppress=noConstructor
        --suppress=useInitializationList
        --suppress=noExplicitConstructor
        --suppress=uselessCallsSubstr
        --suppress=postfixOperator
        --suppress=constVariableReference
        --suppress=useStlAlgorithm
        --suppress=syntaxError
        --error-exitcode=1
        --inline-suppr
        --std=c++20
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      COMMENT "Running cppcheck static analysis..."
    )
  else()
    message(WARNING "CPPCHECK=ON but jq was not found on PATH. cppcheck target unavailable")
  endif()
endif()

