#!/bin/sh

echo ">===Starting_benchmarks==="

runs=10
warmup=10
cache=$((24*1024))
if [ "$1" = "1" ]; then
    /bench/bin/memload 0 $runs $warmup $((512*1024))
elif [ "$1" = "2" ]; then
    /bench/bin/memload 0 100000 $warmup $cache &
    /bench/bin/memload 1 $runs $warmup $((512*1024))
elif [ "$1" = "4" ]; then
    /bench/bin/memload 0 100000 $warmup $cache &
    /bench/bin/memload 1 100000 $warmup $cache &
    /bench/bin/memload 2 100000 $warmup $cache &
    /bench/bin/memload 3 $runs $warmup $((512*1024))
elif [ "$1" = "6" ]; then
    /bench/bin/memload 0 100000 $warmup $cache &
    /bench/bin/memload 1 100000 $warmup $cache &
    /bench/bin/memload 2 100000 $warmup $cache &
    /bench/bin/memload 3 100000 $warmup $cache &
    /bench/bin/memload 4 100000 $warmup $cache &
    /bench/bin/memload 5 $runs $warmup $((512*1024))
else
    echo "UNSUPPORTED!"
fi

echo "<===Benchmarks_done==="
