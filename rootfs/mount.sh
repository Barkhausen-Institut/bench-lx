#!/bin/sh
if [ "$(uname -m)" = "x86_64" ]; then
    mount /dev/sdb /bench
    mount /dev/sdc /benchrun
else
    mount /dev/vdb /bench
    mount /dev/vdc /benchrun
fi
