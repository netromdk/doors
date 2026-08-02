# Freestanding helper library shared by all userland programs.

set(USER_LIB_SOURCES
  Util.cc
)

add_library(userlib STATIC ${USER_LIB_SOURCES})
set_target_properties(userlib PROPERTIES OUTPUT_NAME "user")
target_compile_definitions(userlib PRIVATE __IS_DOORS_USERLAND)
target_compile_options(userlib PRIVATE -ffreestanding -fno-builtin -fno-exceptions -fno-rtti)
target_include_directories(userlib PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/libc++/include
)
