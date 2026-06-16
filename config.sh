SYSTEM_HEADER_PROJECTS="libc++ kernel"
PROJECTS="libc++ kernel"

export PATH="$PWD/bootstrap/bin:$PATH"

export MAKE=${MAKE:-make}
export HOST=${HOST:-$(./scripts/default-host.sh)}

# Number of simultaneous compilation jobs to keep.
export JOBS=${JOBS:-$(nproc)}

export AR=${HOST}-ar
export AS=${HOST}-as
export CC=${HOST}-gcc
export CXX=${HOST}-g++

export AR_HOST=ar
export AS_HOST=as
export CC_HOST=gcc
export CXX_HOST=g++

export PREFIX=/usr
export EXEC_PREFIX=$PREFIX
export BOOTDIR=/boot
export LIBDIR=$EXEC_PREFIX/lib
export INCLUDEDIR=$PREFIX/include

export CFLAGS='-O0'

# When `DEBUG_THROUGH_SERIAL_COM1` is defined, all `printf()` will be output to COM1 (and QEMU
# captures and into a log file via `EMUFLAGS` in the top-level Makefile).
export CPPFLAGS='-DDEBUG_THROUGH_SERIAL_COM1'
#export CPPFLAGS=''

# Configure the cross-compiler to use the desired system root.
#export CC="$CC --sysroot=$PWD/sysroot"
#export CXX="$CXX --sysroot=$PWD/sysroot"

# Work around that the -elf gcc targets doesn't have a system include directory
# because configure received --without-headers rather than --with-sysroot.
if echo "$HOST" | grep -Eq -- '-elf($|-)'; then
    export CC="$CC -isystem=$PWD/sysroot/$INCLUDEDIR"
    export CXX="$CXX -isystem=$PWD/sysroot/$INCLUDEDIR"
fi
