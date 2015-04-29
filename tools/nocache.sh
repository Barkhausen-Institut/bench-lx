#!/bin/sh

if [ $# -ne 2 ]; then
    echo "Usage: $0 <no-for-30cycles> <no-for-13-cycles>" 1>&2
    exit 1
fi

no30=$1
no13=$2

octave -q --eval "A = [1, 13; 1, 30]; b = [$no13; $no30]; x = A \ b; nth_element(x, 2)"
