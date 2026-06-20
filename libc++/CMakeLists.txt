set(LIBCPP_SOURCES
  stdio/putchar.cpp
  stdio/puts.cpp
  assert/assertFail.cpp
  stdlib/abort.cpp
  stdlib/itos.cpp
  stdlib/utos.cpp
  stdlib/ltos.cpp
  stdlib/strtol.cpp
  stdlib/strtoul.cpp
  stdlib/strtoll.cpp
  stdlib/strtoull.cpp
  stdlib/atoi.cpp
  stdlib/atol.cpp
  string/memcpy.cpp
  string/memmove.cpp
  string/memset.cpp
  string/memcmp.cpp
  string/memchr.cpp
  string/strlen.cpp
  string/strcpy.cpp
  string/strncpy.cpp
  string/strcmp.cpp
  string/strncmp.cpp
  string/strchr.cpp
  string/strrchr.cpp
  string/string.cpp
)

# Hosted userland library.
add_library(libcpp STATIC ${LIBCPP_SOURCES})
set_target_properties(libcpp PROPERTIES OUTPUT_NAME "c++")
target_compile_definitions(libcpp PRIVATE __IS_DOORS_LIBCPP)
target_compile_options(libcpp PRIVATE -fno-builtin)
target_include_directories(libcpp PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Freestanding kernel library.
add_library(k STATIC ${LIBCPP_SOURCES})
target_compile_definitions(k PRIVATE
  __IS_DOORS_LIBCPP
  __IS_DOORS_KERNEL)
target_compile_options(k PRIVATE
  -ffreestanding -fno-builtin -fno-exceptions -fno-rtti)
target_include_directories(k PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/kernel/include)

install(
  DIRECTORY include/
  DESTINATION include
)
