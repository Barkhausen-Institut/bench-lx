#!/usr/bin/env bash

set -eu

if [ $# -ne 3 ]; then
    echo "Usage: $0 <image> <directory> <size>" >&2
    exit 1
fi

img="$1"
dir="$2"
size="$3"

rm -f "$img"
mke2fs \
  -L '' \
  -N 0 \
  -O ^64bit \
  -d "$dir" \
  -m 5 \
  -r 1 \
  -t ext2 \
  "$img" \
  "$size"
