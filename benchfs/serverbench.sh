#!/bin/sh

echo ">===Starting_benchmarks==="

if [ "$1" = "nginx" ]; then
    workerpid=`ps aux | grep 'nginx: worker' | xargs | cut -d ' ' -f 1`
    while [ "$workerpid" = "" ]; do
        sleep 1
        workerpid=`ps aux | grep 'nginx: worker' | xargs | cut -d ' ' -f 1`
    done
    echo "Worker pid: $workerpid"
fi

for i in 1 2; do
    /bench/bin/stracetimecontrol set $workerpid &>/dev/null

    for sz in 64 1 1024 2 128 32 4 512 8 256 16; do
        ab -n 3 -c 1 localhost/${sz}k.txt &>/dev/null
    done

    /bench/bin/stracetimecontrol print

    if [ $i -ne 2 ]; then
        echo "############"
    fi
done

echo "<===Benchmarks_done==="
