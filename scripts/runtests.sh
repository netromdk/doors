#!/bin/sh
set -e
. ./config.sh

echo "=== Running tests ==="
DESTDIR="$PWD/sysroot" $MAKE -C tests run
