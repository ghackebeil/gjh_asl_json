gjh_asl_json
============

.. image:: https://travis-ci.org/ghackebeil/gjh_asl_json.svg?branch=master
    :target: https://travis-ci.org/ghackebeil/gjh_asl_json

.. image:: https://codecov.io/github/ghackebeil/gjh_asl_json/coverage.svg?branch=master
    :target: https://codecov.io/github/ghackebeil/gjh_asl_json?branch=master

[![Build Status](https://travis-ci.org/ghackebeil/gjh_asl_json.svg?branch=master)](https://travis-ci.org/ghackebeil/gjh_asl_json)

A simple tool providing similar functionality to that of the gjh
"solver" distributed with the AMPL Solver Library. NLP information
is summarized in a JSON formatted output file.

Installation
~~~~~~~~~~~~

 1. $ cd Thirdparty/
 2. $ ./get.ASL
 3. $ cd ..
 4. $ make

Usage
~~~~~

 1. $ gjh_asl_json stub.nl
 2. $ python
```python
>>> import json
>>> with open('stub.json') as f:
>>>    gjh = json.load(f)
```
