# gjh_asl_json
A tool providing similar functionality to that of the gjh
"solver" distributed with the AMPL Solver Library. Result is
output in JSON format.

Installation looks something like:
 1. $ cd Thirdparty/
 2. $ ./get.ASL
 3. $ cd ..
 4. $ make

Usage:
 * $ gjh_asl_json stub.nl
 * ```python
   import json
   with open('stub.json') as f:
      gjh = json.load(f)
   ```
