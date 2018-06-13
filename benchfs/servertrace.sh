#!/bin/sh

echo ">===Starting_benchmarks==="

rm /tmp/* 2>/dev/null

# create the strace containing just the handling of the requests
if [ "$1" = "nginx" ]; then
    workerpid=`ps aux | grep 'nginx: worker' | xargs | cut -d ' ' -f 1`
    echo "Worker pid: $workerpid"
    strace -o /tmp/res.txt -s 1 -p $workerpid &
    sleep 5
fi

for sz in 64 1 1024 2 128 32 4 512 8 256 16; do
    ab -n 3 -c 1 localhost/${sz}k.txt &>/dev/null
done

cat /tmp/res.txt

echo
echo "<===Benchmarks_done==="
