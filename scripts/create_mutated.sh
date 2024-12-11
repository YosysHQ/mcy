#!/bin/bash

set -e

programname=$( basename "$0" )

function usage {
	echo "$programname: create a mutated module (for use in mcy test scripts)"
	echo "usage: $programname [-h] [-c] [-i infile] [-o outfile] [-s scriptfile]"
	echo "  -h|--help           show this message"
	echo "  -c|--ctrl           add control input 'mutsel' to mutated module"
	echo "  -w|--ctrl-width     width of the control input 'mutsel' to mutated module"
	echo "                        default: 8 bit"
	echo "  -i|--input <file>   file containing mutation information in mcy format"
	echo "                        default: input.txt"
	echo "  -o|--output <file>  name of output file (must end in .v, .sv or .il)"
	echo "                        default: mutated.v"
	echo "  -s|--script <file>  name of script file"
	echo "                        default: mutate.ys"
	echo "  -d|--design <file>  name of design file"
	echo "                        default: ../../database/design.il"
	echo "the yosys log is written to the file <scriptfile>.log"
}

while [[ "$#" -gt 0 ]]; do case $1 in
	-h|--help) usage; exit 0;;
	-c|--ctrl) use_ctrl=1;;
	-w|--ctrl-width) ctrl_width=$2
		if [[ -z "$ctrl_width" || ( ${ctrl_width:0:1} == "-" ) ]]; then
			echo "Missing argument to $1" 1>&2
			exit 1
		fi; shift;;
	-i|--input) input_file=$2
		if [[ -z "$input_file" || ( ${input_file:0:1} == "-" ) ]]; then
			echo "Missing argument to $1" 1>&2
			exit 1
		fi; shift;;
	-o|--output) output_file=$2
		if [[ -z "$output_file" || ( ${output_file:0:1} == "-" ) ]]; then
			echo "Missing argument to $1" 1>&2
			exit 1
		fi; shift;;
	-s|--script) script_file=$2
		if [[ -z "$script_file" || ( ${script_file:0:1} == "-" ) ]]; then
			echo "Missing argument to $1" 1>&2
			exit 1
		fi; shift;;
	-d|--design) design_file=$2
		if [[ -z "$design_file" || ( ${design_file:0:1} == "-" ) ]]; then
			echo "Missing argument to $1" 1>&2
			exit 1
		fi; shift;;
	*) echo "Unrecognized option: $1" 1>&2; usage 1>&2; exit 1;;
esac; shift; done

if [[ -n "$ctrl_width" && $use_ctrl -ne 1 ]]; then
	echo "Warning: control signal width was specified but creation of control input 'mutsel' is not enabled." 1>&2
fi

input_file=${input_file:-input.txt}
output_file=${output_file:-mutated.v}
script_file=${script_file:-mutate.ys}
design_file=${design_file:-../../database/design.il}
ctrl_width=${ctrl_width:-8}

if [[ ( "$output_file" == *.v ) ]]; then
	write_cmd="write_verilog -norename $output_file"
elif [[ ( "$output_file" == *.sv ) ]]; then
	write_cmd="write_verilog -norename -sv $output_file"
elif [[ ( "$output_file" == *.il ) ]]; then
	write_cmd="write_rtlil $output_file"
else
	echo "Unrecognized file extension: '$output_file' (this script can write .v, .sv and .il files)" 1>&2
	# usage 1>&2
	exit 1
fi

{
	echo "read_rtlil $design_file"
	while read -r idx mut; do
		if [[ "$use_ctrl" -eq 1 ]]; then
			echo "mutate -ctrl mutsel ${ctrl_width} ${idx} ${mut#* }"
		else
			echo "mutate ${mut#* }"
			if [[ -n $more_than_one_read ]]; then
				echo "Warning: $input_file contains more than one mutate command but creation of control input 'mutsel' is not enabled." 1>&2
			fi
		fi
		more_than_one_read="yes"
	done < $input_file
	echo "$write_cmd"
} > $script_file

yosys -ql ${script_file%%.ys}.log $script_file
