#!/bin/sh

numa=(0 0-1)

threads=(16 32 64 128 256)

for nodes in ${numa[@]}
do
	numactl -N $nodes ./tbench_srv &
	sleep 1

	for thread in ${threads[@]}
	do
		for i in {1..3}
		do
			numactl -N $nodes bash -x ./start_tbench.sh $thread 120 -numa-$nodes-iter-$i log
		done
	done

	kill `pidof tbench_srv`
done
