#!/bin/bash

exec 2>&1
set -ex

# specify the path to the mutation export script
SCRIPTS=/mnt/data/liyf/Computer-Architecture/model_checking/oss-cad-suite/share/mcy/scripts

## create the mutated design
bash $SCRIPTS/create_mutated.sh -c -o mutated.il

ln -fs ../../fm_smtbmc.sby .

sed -i "s/@TIMEOUT@/$1/" fm_smtbmc.sby

if [ $KEEPDIR = 1 ]; then
	sed -i "/^aigsmt / d;" fm_smtbmc.sby
fi

sby -f fm_smtbmc.sby

## obtain result
gawk "{ print 1, \$1; }" fm_smtbmc/status >> output.txt

exit 0
