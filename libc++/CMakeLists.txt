set(LIBCPP_SOURCES
  assert/assertFail.cc
  stdio/putchar.cc
  stdio/puts.cc
  stdio/format.cc
  stdlib/abort.cc
  stdlib/atoi.cc
  stdlib/atol.cc
  stdlib/itos.cc
  stdlib/ltos.cc
  stdlib/strtol.cc
  stdlib/strtoll.cc
  stdlib/strtoul.cc
  stdlib/strtoull.cc
  stdlib/utos.cc
  string/memchr.cc
  string/memcmp.cc
  string/memcpy.cc
  string/memmove.cc
  string/memset.cc
  string/strchr.cc
  string/strcmp.cc
  string/strcpy.cc
  string/string.cc
  string/strncmp.cc
  string/strncpy.cc
  string/strrchr.cc
)

# Hosted userland library.
add_library(libcpp STATIC ${LIBCPP_SOURCES})
set_target_properties(libcpp PROPERTIES OUTPUT_NAME "c++")
target_compile_definitions(libcpp PRIVATE __IS_DOORS_LIBCPP)
target_compile_options(libcpp PRIVATE -fno-builtin)
target_include_directories(libcpp PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Freestanding userland library for userland programs.
set(LIBCPP_USER_SOURCES ${LIBCPP_SOURCES} stdlib/malloc.cc)
add_library(libcpp_user STATIC ${LIBCPP_USER_SOURCES})
target_compile_definitions(libcpp_user PRIVATE
  __IS_DOORS_LIBCPP
  __IS_DOORS_USERLAND)
target_compile_options(libcpp_user PRIVATE
  -ffreestanding -fno-builtin -fno-exceptions -fno-rtti)
target_include_directories(libcpp_user PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(libcpp_user PROPERTIES OUTPUT_NAME "c++_user")

# Freestanding kernel library.
add_library(libcpp_kernel STATIC ${LIBCPP_SOURCES})
set_target_properties(libcpp_kernel PROPERTIES OUTPUT_NAME "c++_kernel")
target_compile_definitions(libcpp_kernel PRIVATE
  __IS_DOORS_LIBCPP
  __IS_DOORS_KERNEL)
target_compile_options(libcpp_kernel PRIVATE
  -ffreestanding -fno-builtin -fno-exceptions -fno-rtti)
target_include_directories(libcpp_kernel PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/kernel/include)

install(
  DIRECTORY include/
  DESTINATION include
)
