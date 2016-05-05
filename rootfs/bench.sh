#!/bin/sh

echo ">=== Starting benchmarks ==="

/bench/syscall

echo "Cycles per task-create (avg): 0"

/bench/thread
/bench/fork
/bench/exec /bench/noop

/bench/read /starwars.txt 1>/dev/null
/bench/read /starwars.txt > /tmp/res.txt && grep "Read\|Memcpy" /tmp/res.txt

/bench/readmmap /starwars.txt > /tmp/res.txt && grep Read /tmp/res.txt

/bench/readmmapcpy /starwars.txt > /tmp/res.txt && grep Read /tmp/res.txt

/bench/readchksum /starwars.txt > /tmp/res.txt && grep "Read\|Memcpy" /tmp/res.txt

/bench/mmapchksum /starwars.txt > /tmp/res.txt && grep Read /tmp/res.txt

/bench/write /tmp/write.out $((2*1024*1024)) > /tmp/res.txt && grep "Write\|Memcpy" /tmp/res.txt

/bench/cp /starwars.txt /tmp/cp.out > /tmp/res.txt && grep "Write\|Memcpy" /tmp/res.txt

/bench/cpmmap /starwars.txt /tmp/cpmmap.out > /tmp/res.txt && grep Write /tmp/res.txt

/bench/pipe $((2*1024*1024)) > /tmp/res.txt && grep "Read\|Memcpy" /tmp/res.txt

/bench/execpipe /bench/cat /bench/wc /largetext.txt > /tmp/res.txt && grep "Total\|Memcpy" /tmp/res.txt

/bench/pipetr /largetext.txt /tmp/foo.txt a b > /tmp/res.txt && grep "Total\|Memcpy\|App" /tmp/res.txt

/bench/fftpipe /tmp/fft.out > /tmp/res.txt && grep "Total\|Memcpy" /tmp/res.txt

echo "<=== Benchmarks done ==="
