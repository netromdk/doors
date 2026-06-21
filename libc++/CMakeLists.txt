set(LIBCPP_SOURCES
  stdio/putchar.cc
  stdio/puts.cc
  assert/assertFail.cc
  stdlib/abort.cc
  stdlib/itos.cc
  stdlib/utos.cc
  stdlib/ltos.cc
  stdlib/strtol.cc
  stdlib/strtoul.cc
  stdlib/strtoll.cc
  stdlib/strtoull.cc
  stdlib/atoi.cc
  stdlib/atol.cc
  string/memcpy.cc
  string/memmove.cc
  string/memset.cc
  string/memcmp.cc
  string/memchr.cc
  string/strlen.cc
  string/strcpy.cc
  string/strncpy.cc
  string/strcmp.cc
  string/strncmp.cc
  string/strchr.cc
  string/strrchr.cc
  string/string.cc
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
