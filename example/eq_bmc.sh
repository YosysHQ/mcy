#!/bin/bash

option_cleanup_workdir=true

set -e
mkdir "tempdir/task_$TASK"
cd "tempdir/task_$TASK"
cat > input.txt

(
	set -ex

	{
		echo "read_ilang ../../database/design.il"
		while read idx mut; do
			idx=${idx%:}
			echo "mutate -ctrl mutsel 8 ${idx} ${mut#* }"
		done < input.txt
		echo "write_ilang mutated.il"
	} > mutate.ys

	yosys -ql mutate.log mutate.ys
	cp ../../miter.sv ../../eq_bmc.sby .

	while read idx mut; do
		idx=${idx%:}
		sby -f eq_bmc.sby ${idx}
		gawk "{ print $idx \":\", \$1; }" eq_bmc_${idx}/status >> output.txt
	done < input.txt
) > logfile.txt 2>&1

cat output.txt

if $option_cleanup_workdir; then
	rm -rf "../task_${TASK}"
fi

exit 0
