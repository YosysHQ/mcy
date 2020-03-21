#!/bin/bash

exec 2>&1
set -ex

## create yosys script with instructions how to export the mutated design
{
	# read synthesized design
	echo "read_ilang ../../database/design.il"
	while read -r idx mut; do
		# add mutation to the design, to be enabled by value ${idx} on 8-bit input `mutsel` added to the module
		echo "mutate -ctrl mutsel 8 ${idx} ${mut#* }"
	done < input.txt
	# export design to RTLIL
	echo "write_ilang mutated.il"
} > mutate.ys

## run the above script to create mutated.il
yosys -ql mutate.log mutate.ys

## run formal property check
ln -s ../../test_eq.sv ../../test_eq.sby .
sby -f test_eq.sby

## obtain result
gawk "{ print 1, \$1; }" test_eq/status >> output.txt

exit 0
