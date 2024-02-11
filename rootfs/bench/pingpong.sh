#!/bin/sh

echo ">===Starting_benchmarks==="

msgsize=$1
if [ "$2" = "2" ]; then
    /bench/bin/pingpong 0 1 1 "$msgsize"
elif [ "$2" = "4" ]; then
    /bench/bin/pingpong 0 1 10 "$msgsize" &
    /bench/bin/pingpong 2 3 1 "$msgsize"
elif [ "$2" = "6" ]; then
    /bench/bin/pingpong 0 1 10 "$msgsize" &
    /bench/bin/pingpong 2 3 10 "$msgsize" &
    /bench/bin/pingpong 4 5 1 "$msgsize"
else
    echo "UNSUPPORTED!"
fi

echo "<===Benchmarks_done==="
