#!/bin/sh
set -e
. ./scripts/headers.sh

echo "=== Building test libc++ ==="
DESTDIR="$PWD/sysroot" $MAKE -j ${JOBS} -C libc++ test install-tests
