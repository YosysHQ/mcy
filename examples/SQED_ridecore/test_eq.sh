#!/bin/bash

exec 2>&1
set -ex

SCRIPTS=/mnt/data/liyf/Computer-Architecture/model_checking/oss-cad-suite/share/mcy/scripts

bash $SCRIPTS/create_mutated.sh -c -o mutated.il

ln -fs ../../test_eq.sby .
sby -f test_eq.sby

## obtain result
gawk "{ print 1, \$1; }" test_eq/status >> output.txt

exit 0