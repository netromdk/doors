#!/bin/sh
TEST=$1
PWD=`pwd`

. ./testconfig.sh

echo "=== Running test: $TEST ==="
RET=`$PWD/$TEST; echo $?`
if [ $RET -eq 0 ]; then
    echo "YES" >> ${CNTFILE}
else
    echo "NO" >> ${CNTFILE}
    echo "FAILED"
    echo "Result: $RET"
fi
