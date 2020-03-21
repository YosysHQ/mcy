#!/bin/bash

exec 2>&1
set -ex

## create yosys script with instructions how to export the mutated design
{
	# read synthesized design
	echo "read_ilang ../../database/design.il"
	while read -r idx mut; do
		# add mutation to the design (always enabled)
		echo "mutate ${mut#* }"
	done < input.txt
	# export design to verilog
	echo "write_verilog -attr2comment mutated.v"
} > mutate.ys

## run the above script to create mutated.v
yosys -ql mutate.log mutate.ys

## run the testbench with the mutated module substituted for the original
iverilog -o sim ../../bitcnt_tb.v mutated.v
vvp -n sim > sim.out

## check simulation output to obtain result status
if grep PASS sim.out && ! grep ERROR sim.out; then
	echo "1 PASS" > output.txt
elif ! grep PASS sim.out && grep ERROR sim.out; then
	echo "1 FAIL" > output.txt
else
	echo "1 ERROR" > output.txt
fi

exit 0
