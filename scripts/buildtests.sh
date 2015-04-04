#!/bin/sh
set -e
. ./scripts/headers.sh

echo "=== Building test libc++ ==="
DESTDIR="$PWD/sysroot" $MAKE -C libc++ test install-tests
