#!/bin/bash

exec 2>&1
set -ex

# specify the path to the mutation export script
SCRIPTS=/mnt/data/liyf/Computer-Architecture/model_checking/oss-cad-suite/share/mcy/scripts

## create the mutated design
bash $SCRIPTS/create_mutated.sh -o mutated.il

## run formal property check
ln -fs ../../fm_btor.sby .
sby -f fm_btor.sby

## obtain result
gawk "{ print 1, \$1; }" fm_btor/status >> output.txt

exit 0
