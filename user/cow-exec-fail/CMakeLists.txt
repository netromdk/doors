add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/cow-exec-fail.elf"
  COMMAND
    ${CMAKE_CXX_COMPILER}
    -Wall -Wextra -Wpedantic -Werror
    -ffreestanding -nostdlib -static -no-pie -std=c++20
    -T "${CMAKE_CURRENT_SOURCE_DIR}/link.ld"
    -I "${CMAKE_CURRENT_SOURCE_DIR}/.."
    -I "${CMAKE_SOURCE_DIR}/libc++/include"
    -fno-exceptions -fno-rtti -fno-builtin
    -o "${CMAKE_CURRENT_BINARY_DIR}/cow-exec-fail.elf"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cc"
  DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/link.ld"
  VERBATIM
)

add_custom_target(cow-exec-fail ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/cow-exec-fail.elf")

if(NOT TARGET user_programs)
  add_custom_target(user_programs)
endif()
add_dependencies(user_programs cow-exec-fail)
