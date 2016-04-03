#! /bin/bash

# These are simple tests that mostly serve
# to generate a code coverage report.

set -e

rm -f stub1.json
rm -f stub1.sol

# stub1.nl
! test -f stub1.json
cmd="../bin/gjh_asl_json stub1.nl"
echo "Testing command: ${cmd}"
${cmd}
test -f stub1.json
python -c "import json; f = open('stub1.json'); json.load(f); f.close()"
rm -f stub1.json

# stub1.nl with -s
! test -f stub1.json
! test -f stub1.sol
cmd="../bin/gjh_asl_json -s stub1.nl"
echo "Testing command: ${cmd}"
${cmd}
test -f stub1.json
test -f stub1.sol
python -c "import json; f = open('stub1.json'); json.load(f); f.close()"
rm -f stub1.json
rm -f stub1.sol

# stub1.nl with row and col labels
! test -f stub1.json
cmd="../bin/gjh_asl_json stub1.nl rows=stub1.row cols=stub1.col"
echo "Testing command: ${cmd}"
${cmd}
test -f stub1.json
python -c "import json; f = open('stub1.json'); json.load(f); f.close()"
rm -f stub1.json

# stub2.nl
rm -f stub2.json
! test -f stub2.json
cmd="../bin/gjh_asl_json stub2.nl"
echo "Testing command: ${cmd}"
${cmd}
test -f stub2.json
python -c "import json; f = open('stub2.json'); json.load(f); f.close()"

rm -f stub2.json
