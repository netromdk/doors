#!/bin/sh
set -e
. ./config.sh

echo "=== Building and running tests ==="
DESTDIR="$PWD/sysroot" $MAKE -j ${JOBS} -C tests run
