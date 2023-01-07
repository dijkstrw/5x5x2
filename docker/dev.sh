#!/bin/bash
if [ $# -eq 0 ]; then
    args=""
    opts=""
    interactive=-it
else
    args=$*
    opts="-l -c"
    interactive=""
fi
docker run --rm --privileged $interactive -v `pwd`:/mnt -- stm32-gcc bash $opts ${args:+"$args"}
