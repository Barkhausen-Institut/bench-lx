#!/bin/sh

echo ">===Starting_benchmarks==="

/bench/bin/syscall

echo "Cycles per task-create (avg): 0"

/bench/bin/thread
/bench/bin/fork
/bench/bin/exec /bench/bin/noop

/bench/bin/read /bench/large.bin 1>/dev/null
/bench/bin/read /bench/large.bin > /tmp/res.txt && grep "Read\|Memcpy" /tmp/res.txt

/bench/bin/readmmap /bench/large.bin > /tmp/res.txt && grep Read /tmp/res.txt

/bench/bin/readmmapcpy /bench/large.bin > /tmp/res.txt && grep Read /tmp/res.txt

/bench/bin/readchksum /bench/large.bin > /tmp/res.txt && grep "Read\|Memcpy" /tmp/res.txt

/bench/bin/mmapchksum /bench/large.bin > /tmp/res.txt && grep Read /tmp/res.txt

/bench/bin/write /tmp/write.out $((2*1024*1024)) > /tmp/res.txt && grep "Write\|Memcpy" /tmp/res.txt

/bench/bin/cp /bench/large.bin /tmp/cp.out > /tmp/res.txt && grep "Write\|Memcpy" /tmp/res.txt

/bench/bin/cpmmap /bench/large.bin /tmp/cpmmap.out > /tmp/res.txt && grep Write /tmp/res.txt

/bench/bin/pipe $((2*1024*1024)) > /tmp/res.txt && grep "Read\|Memcpy" /tmp/res.txt

/bench/bin/execpipe /bench/bin/cat /bench/bin/wc /bench/large.txt > /tmp/res.txt && grep "Total\|Memcpy" /tmp/res.txt

/bench/bin/pipetr /bench/large.txt /tmp/foo.txt a b > /tmp/res.txt && grep "Total\|Memcpy\|App" /tmp/res.txt

/bench/bin/fftpipe /tmp/fft.out > /tmp/res.txt && grep "Total\|Memcpy" /tmp/res.txt

/bench/bin/yield > /tmp/res.txt && grep "Time" /tmp/res.txt

# /bench/bin/pagefault > /tmp/res.txt && grep "Time" /tmp/res.txt

echo "<===Benchmarks_done==="
