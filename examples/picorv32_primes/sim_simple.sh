#!/bin/bash

option_run_unmodified=false

exec 2>&1
set -ex

{
	echo "read_rtlil ../../database/design.il"
	while read -r idx mut; do
		echo "mutate -ctrl mutsel 8 ${idx} ${mut#* }"
	done < input.txt
	echo "write_verilog -attr2comment mutated.v"
} > mutate.ys

yosys -ql mutate.log mutate.ys
iverilog -o sim ../../sim_simple.v mutated.v

if $option_run_unmodified; then
	vvp -N sim +mut=0 > sim_unmodified.out
	good_md5sum=$(md5sum sim_unmodified.out | awk '{ print $1; }')
else
	good_md5sum=58bdae1d2a140fde0fcff8d8a743e62f
fi

while read idx mut; do
	vvp -N sim +mut=${idx} > sim_${idx}.out
	this_md5sum=$(md5sum sim_${idx}.out | awk '{ print $1; }')
	if [ $good_md5sum = $this_md5sum ]; then
		echo "$idx PASS" >> output.txt
	else
		echo "$idx FAIL" >> output.txt
	fi
done < input.txt

exit 0
