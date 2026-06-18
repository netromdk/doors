set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i386)

get_filename_component(_DOORS_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(_BOOTSTRAP_BIN "${_DOORS_ROOT}/bootstrap/bin")

if (EXISTS "${_BOOTSTRAP_BIN}/i386-elf-g++")
  set(CMAKE_C_COMPILER   "${_BOOTSTRAP_BIN}/i386-elf-gcc"    CACHE FILEPATH "C compiler"   FORCE)
  set(CMAKE_CXX_COMPILER "${_BOOTSTRAP_BIN}/i386-elf-g++"    CACHE FILEPATH "C++ compiler" FORCE)
  set(CMAKE_ASM_COMPILER "${_BOOTSTRAP_BIN}/i386-elf-gcc"    CACHE FILEPATH "ASM compiler" FORCE)
  set(CMAKE_AR           "${_BOOTSTRAP_BIN}/i386-elf-ar"     CACHE FILEPATH "Archiver"     FORCE)
  set(CMAKE_RANLIB       "${_BOOTSTRAP_BIN}/i386-elf-ranlib" CACHE FILEPATH "Ranlib"       FORCE)
else()
  find_program(_CROSS_GXX i386-elf-g++ NO_CACHE)
  if (_CROSS_GXX)
    set(CMAKE_C_COMPILER   "i386-elf-gcc"    CACHE FILEPATH "C compiler"   FORCE)
    set(CMAKE_CXX_COMPILER "i386-elf-g++"    CACHE FILEPATH "C++ compiler" FORCE)
    set(CMAKE_ASM_COMPILER "i386-elf-gcc"    CACHE FILEPATH "ASM compiler" FORCE)
    set(CMAKE_AR           "i386-elf-ar"     CACHE FILEPATH "Archiver"     FORCE)
    set(CMAKE_RANLIB       "i386-elf-ranlib" CACHE FILEPATH "Ranlib"       FORCE)
  else()
    message(FATAL_ERROR
      "i386-elf cross-compiler not found.\n"
      "  Expected: ${_BOOTSTRAP_BIN}/i386-elf-g++\n"
      "Build the toolchain first:\n"
      "  ./${_DOORS_ROOT}/scripts/bootstrap.sh\n"
    )
  endif()
endif()

# Prevent CMake's compiler-detection try-compile from attempting to link a runnable executable.
# The cross-compiler cannot produce host-runnable binaries.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_SYSROOT "${_DOORS_ROOT}/sysroot")
set(CMAKE_FIND_ROOT_PATH "${_DOORS_ROOT}/sysroot")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
