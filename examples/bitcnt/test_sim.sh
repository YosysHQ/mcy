#!/bin/bash

exec 2>&1
set -ex

## create the mutated design
bash $SCRIPTS/create_mutated.sh

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
