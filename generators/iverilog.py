#!/usr/bin/env python3

import getopt, os

def usage():
    print()
    print("Usage:")
    print("  mcy generate iverilog -t test.v -o sim.sh -m [md5_of_output]")
    print()
    exit(1)

def generator_main(name, argv):
    try:
        opts, args = getopt.getopt(argv, 't:o:m:', ['tb=', 'output=', 'md5='])
    except getopt.GetoptError as err:
        print(err)
        usage()

    if len(argv) < 1:
        usage()
    
    tb_path = ''
    sh_output = ''
    compare_md5 = False    
    for opt, arg in opts:
        if opt in ('-t', '--tb'):
            tb_path = arg
        if opt in ('-o', '--output'):
            sh_output = arg
        if opt in ('-m', '--md5'):
            md5 = arg
            compare_md5 = True

    if not sh_output:
        print("output parameter must be specified !!!")
        exit(1)

    if not tb_path:
        print("tb parameter must be specified !!!")
        exit(1)

    with open(sh_output, "w") as f:
        print("#!/bin/bash", file=f)
        print("", file=f)
        print("exec 2>&1", file=f)
        print("set -ex", file=f)
        print("", file=f)
        print("{", file=f)
        print("     echo \"read_ilang ../../database/design.il\"", file=f)
        print("     while read -r idx mut; do", file=f)
        print("         echo \"mutate -ctrl mutsel 8 ${idx} ${mut#* }\"", file=f)
        print("     done < input.txt", file=f)
        print("     echo \"write_verilog -attr2comment mutated.v\"", file=f)
        print("} > mutate.ys", file=f)
        print("", file=f)
        print("yosys -ql mutate.log mutate.ys", file=f)
        print("", file=f)
        print("iverilog -o sim ../../" + tb_path + " mutated.v", file=f)
        print("", file=f)
        if compare_md5:
            print("good_md5sum=" + md5 , file=f)
            print("", file=f)
            print("while read idx mut; do", file=f)
            print("    vvp -N sim +mut=${idx} > sim_${idx}.out", file=f)
            print("    this_md5sum=$(md5sum sim_${idx}.out | awk '{ print $1; }')", file=f)
            print("    if [ $good_md5sum = $this_md5sum ]; then", file=f)
            print("        echo \"$idx PASS\" >> output.txt", file=f)
            print("    else", file=f)
            print("        echo \"$idx FAIL\" >> output.txt", file=f)
            print("    fi", file=f)
            print("done < input.txt", file=f)
    os.chmod(sh_output, 0o744)

    exit(0)
