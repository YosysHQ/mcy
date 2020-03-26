#!/bin/bash

exec 2>&1
set -ex

## create yosys script with instructions how to export the mutated design
{
	# read synthesized design
	echo "read_ilang ../../database/design.il"
	# apply mutation
	cut -f2- -d' ' input.txt
	# export design to RTLIL
	echo "write_ilang mutated.il"
} > mutate.ys

## run the above script to create mutated.il
yosys -ql mutate.log mutate.ys

## run formal property check
ln -s ../../test_fm.sv ../../test_fm.sby .
sby -f test_fm.sby

## obtain result
gawk "{ print 1, \$1; }" test_fm/status >> output.txt

exit 0
