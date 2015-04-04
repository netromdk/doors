#!/bin/sh
. ./testconfig.sh

YES=$(grep -i "yes" ${CNTFILE} | wc -l | awk '{print $1}')
NO=$(grep -i "no" ${CNTFILE} | wc -l | awk '{print $1}')
NUM=$(($YES+$NO))

echo "\n=== Summary ==="
echo "Tests:     $NUM"
echo "Succeeded: $YES"
echo "Failed:    $NO"

if [ $NO -eq 0 ]; then
    echo "ALL GOOD!"
else
    echo "FIX THE BUGS!"
fi

rm -f ${CNTFILE}
