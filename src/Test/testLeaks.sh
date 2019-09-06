#!/bin/bash

rm -f valgrind_out
/usr/bin/valgrind --leak-check=full ./mainFib.out 4 1000 10 2 3 4 100 -s true >& ./valgrind_out

r=$(tail -10 ./valgrind_out | grep "ERROR SUMMARY" | cut -d: -f 2 | cut -d" " -f 2)

if [[ $r != 0 ]]; then
    echo "Test FALLITO"
    exit 1
fi

echo "Test OK!"
exit 0