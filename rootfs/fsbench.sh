#!/bin/sh

echo ">=== Starting benchmarks ==="

strace -o /tmp/res.txt $1 2>/dev/null
cat /tmp/res.txt

echo "===="

rm /tmp/test.db 2>/dev/null
/bench/stracetime $1 2>/dev/null

echo "<=== Benchmarks done ==="
