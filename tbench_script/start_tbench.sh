#!/bin/sh

kernel=`uname -r`
suffix=""
log=false

threads=32
timeout=300

if [ $# -ge 1 ]; then
        threads=$1
fi

if [ $# -gt 1 ]; then
        timeout=$2
fi

if [ $# -gt 2 ]; then
        suffix=$3
fi

if [ $# -gt 3 ]; then
        log=true
fi

if $log; then
        ./tbench --backend=sockio \
         --loadfile=loadfiles/client.txt \
         -t $timeout \
         --server=127.0.0.1 \
         $threads | tee result/$kernel-tbench-$threads$suffix.dat
else
        ./tbench --backend=sockio \
         --loadfile=loadfiles/client.txt \
         -t $timeout \
         --server=127.0.0.1 \
         $threads
fi
