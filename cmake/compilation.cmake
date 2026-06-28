set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-O0 -Wall -Wextra -Wpedantic)

if (VERBOSE_BUILD)
  set(CMAKE_VERBOSE_MAKEFILE ON)
  message(STATUS "Verbose build: ON")
else()
  message(STATUS "Verbose build: OFF")
endif()

if (SERIAL_DEBUG OR KERNEL_UBSAN)
  add_compile_definitions(DEBUG_THROUGH_SERIAL_COM1)
  message(STATUS "Serial debug: ON (printf mirrored to COM1 -> doors.log)")
else()
  message(STATUS "Serial debug: OFF")
endif()

# Host compiler is used for tests and any other host-compiled targets.
if (HOST_CXX_COMPILER)
  find_program(DOORS_HOST_CXX "${HOST_CXX_COMPILER}" NO_CACHE)
  if (NOT DOORS_HOST_CXX)
    message(FATAL_ERROR
      "\n"
      "HOST_CXX_COMPILER is set to '${HOST_CXX_COMPILER}' but it was not found on PATH.\n"
    )
  endif()
else()
  find_program(DOORS_HOST_CXX NAMES clang++ g++ NO_CACHE)
endif()

if (DOORS_HOST_CXX)
  message(STATUS "Host C++ compiler: ${DOORS_HOST_CXX}")
else()
  message(STATUS "Host C++ compiler: not found (tests will be unavailable)")
endif()

if (SANITIZERS)
  message(STATUS "Sanitizers: ${SANITIZERS}")
else()
  message(STATUS "Sanitizers: none")
endif()

if (KERNEL_UBSAN)
  message(STATUS "Kernel UBSan: ON")
else()
  message(STATUS "Kernel UBSan: OFF")
endif()

if (CAPS_LOCK_IS_CTRL)
  add_compile_definitions(CAPS_LOCK_IS_CTRL)
  message(STATUS "Caps Lock remapped as Ctrl: YES")
else()
  message(STATUS "Caps Lock remapped as Ctrl: NO")
endif()
