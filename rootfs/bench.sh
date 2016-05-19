#!/bin/sh

echo ">=== Starting benchmarks ==="

/bench/syscall

echo "Cycles per task-create (avg): 0"

/bench/thread
/bench/fork
/bench/exec /bench/noop

/bench/read /large.bin 1>/dev/null
/bench/read /large.bin > /tmp/res.txt && grep "Read\|Memcpy" /tmp/res.txt

/bench/readmmap /large.bin > /tmp/res.txt && grep Read /tmp/res.txt

/bench/readmmapcpy /large.bin > /tmp/res.txt && grep Read /tmp/res.txt

/bench/readchksum /large.bin > /tmp/res.txt && grep "Read\|Memcpy" /tmp/res.txt

/bench/mmapchksum /large.bin > /tmp/res.txt && grep Read /tmp/res.txt

/bench/write /tmp/write.out $((2*1024*1024)) > /tmp/res.txt && grep "Write\|Memcpy" /tmp/res.txt

/bench/cp /large.bin /tmp/cp.out > /tmp/res.txt && grep "Write\|Memcpy" /tmp/res.txt

/bench/cpmmap /large.bin /tmp/cpmmap.out > /tmp/res.txt && grep Write /tmp/res.txt

/bench/pipe $((2*1024*1024)) > /tmp/res.txt && grep "Read\|Memcpy" /tmp/res.txt

/bench/execpipe /bench/cat /bench/wc /large.txt > /tmp/res.txt && grep "Total\|Memcpy" /tmp/res.txt

/bench/pipetr /large.txt /tmp/foo.txt a b > /tmp/res.txt && grep "Total\|Memcpy\|App" /tmp/res.txt

/bench/fftpipe /tmp/fft.out > /tmp/res.txt && grep "Total\|Memcpy" /tmp/res.txt

echo "<=== Benchmarks done ==="
