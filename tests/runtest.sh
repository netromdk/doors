#!/bin/sh
TEST=$1
PWD=`pwd`

echo "=== Running test: $TEST ==="
RET=`$PWD/$TEST; echo $?`
if [ $RET -eq 0 ]; then
    echo "Success: YES"
else
    echo "Success: NO"
    echo "Result: $RET"
fi
