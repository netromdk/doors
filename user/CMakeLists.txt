include(${CMAKE_SOURCE_DIR}/cmake/user-programs.cmake)

add_subdirectory(shell)
add_subdirectory(snake)

if (BUILD_INTEGRATION_TESTS)
  add_subdirectory(testrunner)
endif ()
