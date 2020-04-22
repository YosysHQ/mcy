#!/bin/bash

exec 2>&1
set -ex

## create the mutated design
bash $SCRIPTS/create_mutated.sh -o mutated.il

## run formal property check
ln -s ../../test_fm.sv ../../test_fm.sby .
sby -f test_fm.sby

## obtain result
gawk "{ print 1, \$1; }" test_fm/status >> output.txt

exit 0
