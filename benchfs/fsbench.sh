#!/bin/sh

echo ">===Starting_benchmarks==="

strace -o /tmp/res.txt $1 2>/dev/null
cat /tmp/res.txt

echo "===="

rm /tmp/test.db 2>/dev/null
/bench/bin/stracetime $1 2>/dev/null

echo "<===Benchmarks_done==="
