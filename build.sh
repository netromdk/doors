#!/bin/sh
set -e
. ./headers.sh

for PROJECT in $PROJECTS; do
  echo "=== Building $PROJECT ==="
  DESTDIR="$PWD/sysroot" $MAKE -C $PROJECT install
done
