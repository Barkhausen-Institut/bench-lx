#!/bin/sh

if [ "`grep "Fake M5" /proc/cpuinfo`" != "" ]; then
    stats=1
else
    stats=0
fi

echo ">===Starting_benchmarks==="

/bench/bin/execpipe /bench/bin/rand $((64*1024)) /bench/bin/wc 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/rand $((128*1024)) /bench/bin/wc 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/rand $((256*1024)) /bench/bin/wc 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/rand $((512*1024)) /bench/bin/wc 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/rand $((1024*1024)) /bench/bin/wc 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt

/bench/bin/execpipe /bench/bin/cat /pipedata/64k.txt /bench/bin/wc 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/cat /pipedata/128k.txt /bench/bin/wc 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/cat /pipedata/256k.txt /bench/bin/wc 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/cat /pipedata/512k.txt /bench/bin/wc 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/cat /pipedata/1024k.txt /bench/bin/wc 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt

/bench/bin/execpipe /bench/bin/rand $((64*1024)) /bench/bin/sink 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/rand $((128*1024)) /bench/bin/sink 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/rand $((256*1024)) /bench/bin/sink 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/rand $((512*1024)) /bench/bin/sink 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/rand $((1024*1024)) /bench/bin/sink 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt

/bench/bin/execpipe /bench/bin/cat /pipedata/64k.txt /bench/bin/sink 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/cat /pipedata/128k.txt /bench/bin/sink 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/cat /pipedata/256k.txt /bench/bin/sink 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/cat /pipedata/512k.txt /bench/bin/sink 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt
/bench/bin/execpipe /bench/bin/cat /pipedata/1024k.txt /bench/bin/sink 2 1 $stats > /tmp/res.txt && grep execpipe /tmp/res.txt

echo "<===Benchmarks_done==="
