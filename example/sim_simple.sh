#!/bin/bash

option_run_unmodified=false
option_cleanup_workdir=true

mkdir "task_$TASK"
cd "task_$TASK"
cat > input.txt

{
	set -ex

	{
		echo "read_ilang ../database/design.il"
		while read idx mut; do
			idx=${idx%:}
			echo "mutate -ctrl mutsel 8 ${idx} ${mut#* }"
		done < input.txt
		echo "write_verilog -attr2comment mutate.v"
	} > mutate.ys

	yosys -ql mutate.log mutate.ys
	iverilog -o sim ../sim_simple.v mutate.v

	if $option_run_unmodified; then
		vvp -N sim +mut=0 > sim_unmodified.out
		good_md5sum=$(md5sum sim_unmodified.out | awk '{ print $1; }')
	else
		good_md5sum=58bdae1d2a140fde0fcff8d8a743e62f
	fi

	while read idx mut; do
		idx=${idx%:}
		vvp -N sim +mut=${idx} > sim_${idx}.out
		this_md5sum=$(md5sum sim_${idx}.out | awk '{ print $1; }')
		if [ $good_md5sum = $this_md5sum ]; then
			echo "$idx: PASS" >> output.txt
		else
			echo "$idx: FAIL" >> output.txt
		fi
	done < input.txt
} > logfile.txt 2>&1

cat output.txt

if $option_cleanup_workdir; then
	rm -rf "../task_${TASK}"
fi

exit 0
