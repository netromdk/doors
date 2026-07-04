add_custom_command(
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/shell.elf"
  COMMAND ${CMAKE_CXX_COMPILER}
    -ffreestanding -nostdlib -static -no-pie
    -T "${CMAKE_CURRENT_SOURCE_DIR}/user.ld"
    -fno-exceptions -fno-rtti -fno-builtin
    -o "${CMAKE_CURRENT_BINARY_DIR}/shell.elf"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shell.cc"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Shell.cc"
          "${CMAKE_CURRENT_SOURCE_DIR}/user.ld"
  VERBATIM
)

add_custom_target(user_programs ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shell.elf")
