#!/bin/sh
# The following directories will be traversed to generate tags from.
DIRS="kernel libc"

# Do an incremental update if already present.
ARG=""
CHK=$(global -q -p; echo $?)
if [ "$CHK" != "3" ]; then
    ARG="-i"
    echo "Updating tags.."
else
    echo "Generating tags.."
fi

# This we are using C++ setting "GTAGSFORCECPP=1" will make gtags
# treat .h as C++ source.
find ${DIRS} -type f | GTAGSFORCECPP=1 gtags ${ARG} -c -f -
