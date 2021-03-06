#!/bin/sh
set -e
. ./config.sh

for PROJECT in $PROJECTS; do
  $MAKE -C $PROJECT clean
done

rm -rf sysroot
rm -fv *.iso *.zip *.tgz *.bz2 *.xz
find . -iname '*~' | xargs rm -rfv
