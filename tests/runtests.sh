#! /bin/bash

# These are simple tests that mostly serve
# to generate a code coverage report.

set -e

# stub1.nl
rm -f stub1.json
! test -f stub1.json
../bin/gjh_asl_json stub1.nl
test -f stub1.json

# stub1.nl with row and col labels
rm -f stub1.json
! test -f stub1.json
../bin/gjh_asl_json stub1.nl rows=stub1.row cols=stub1.col
test -f stub1.json

rm -f stub1.json

# stub2.nl
rm -f stub2.json
! test -f stub2.json
../bin/gjh_asl_json stub2.nl
test -f stub2.json

rm -f stub2.json
