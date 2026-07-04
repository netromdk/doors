function(add_user_program name)
  set(sources)
  set(headers)
  foreach(f ${ARGN})
    if(f MATCHES "\\.h$")
      list(APPEND headers "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
    else()
      list(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
    endif()
  endforeach()

  add_custom_command(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/${name}.elf"
    COMMAND
      ${CMAKE_CXX_COMPILER}
      -ffreestanding -nostdlib -static -no-pie
      -T "${CMAKE_CURRENT_SOURCE_DIR}/../User.ld"
      -I "${CMAKE_CURRENT_SOURCE_DIR}/.."
      -I "${CMAKE_SOURCE_DIR}/libc++/include"
      -fno-exceptions -fno-rtti -fno-builtin
      -o "${CMAKE_CURRENT_BINARY_DIR}/${name}.elf"
      ${sources}
      -L "${CMAKE_BINARY_DIR}/libc++"
      -lcpp_user
    DEPENDS
      ${sources}
      ${headers}
      "${CMAKE_CURRENT_SOURCE_DIR}/../User.ld"
    VERBATIM
  )

  add_custom_target("${name}" ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${name}.elf")
  add_dependencies("${name}" libcpp_user)

  if(NOT TARGET user_programs)
    add_custom_target(user_programs)
  endif()
  add_dependencies(user_programs "${name}")
endfunction()
