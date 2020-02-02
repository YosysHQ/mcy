#!/bin/bash

exec 2>&1
set -ex

{
	echo "read_ilang ../../database/design.il"
	while read -r idx mut; do
		echo "mutate ${mut#* }"
	done < input.txt
	echo "write_verilog -attr2comment mutated.v"
} > mutate.ys

yosys -ql mutate.log mutate.ys

iverilog -o sim ../../bitcnt_tb.v mutated.v
vvp -n sim > sim.out

if grep PASS sim.out && ! grep ERROR sim.out; then
	echo "1 PASS" > output.txt
elif ! grep PASS sim.out && grep ERROR sim.out; then
	echo "1 FAIL" > output.txt
else
	echo "1 ERROR" > output.txt
fi

exit 0
