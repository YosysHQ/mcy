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
		echo "write_ilang mutated.il"
	} > mutate.ys

	yosys -ql mutate.log mutate.ys
	cp ../miter.sv ../eq_sim3.sby .

	while read idx mut; do
		idx=${idx%:}
		sby -f eq_sim3.sby ${idx}
		gawk "{ print $idx, \$1; }" eq_sim3_${idx}/status
	done < input.txt
} > logfile.txt 2>&1

cat output.txt

if $option_cleanup_workdir; then
	rm -rf "../task_${TASK}"
fi

exit 0
