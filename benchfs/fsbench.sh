#!/bin/sh

echo ">===Starting_benchmarks==="

strace -o /tmp/res.txt $1 2>/dev/null
cat /tmp/res.txt

echo "===="

for i in 1 2 3 4; do
    rm -r /tmp/* 2>/dev/null
    /bench/bin/stracetime $1 2>/dev/null

    if [ $i -ne 4 ]; then
        echo "############"
    fi
done

echo "<===Benchmarks_done==="
