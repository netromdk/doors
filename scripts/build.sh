#!/bin/sh
set -e
. ./scripts/headers.sh

for PROJECT in $PROJECTS; do
  echo "=== Building $PROJECT ==="
  DESTDIR="$PWD/sysroot" $MAKE -j ${JOBS} -C $PROJECT install
done
