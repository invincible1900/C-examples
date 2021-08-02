#!/bin/bash
make 
./sig-test &
for((i=1;i<=64;i++));
do
	if (( $i == 9 || $i == 19 || $i == 32 || $i == 33 )); then
		continue
	fi
	kill -$i $(pgrep sig-test)
	#sleep 1
done
kill -9 $(pgrep sig-test)
