#!/bin/sh

echo ">===Starting_benchmarks==="

strace -o /tmp/res.txt $1 2>/dev/null
cat /tmp/res.txt

echo "===="

for i in 1 2 3 4 5 6 7 8; do
    rm /tmp/test.db 2>/dev/null
    /bench/bin/stracetime $1 2>/dev/null

    if [ $i -ne 8 ]; then
        echo "############"
    fi
done

echo "<===Benchmarks_done==="
