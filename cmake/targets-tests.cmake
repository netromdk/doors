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
    "-DDOORS_LIBC_SRC_DIR=${CMAKE_SOURCE_DIR}/libc++"
    "-DDOORS_LIBC_INC_DIR=${CMAKE_SOURCE_DIR}/libc++/include"
  CMAKE_CACHE_ARGS "-DSANITIZERS:STRING=${SANITIZERS}"
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
