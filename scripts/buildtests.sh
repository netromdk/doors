#!/bin/sh
set -e
. ./scripts/headers.sh

echo "=== Building tests ==="
DESTDIR="$PWD/sysroot" $MAKE -C libc test install-tests
