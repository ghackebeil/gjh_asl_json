gjh_asl_json
============

.. image:: https://travis-ci.org/ghackebeil/gjh_asl_json.svg?branch=master
    :target: https://travis-ci.org/ghackebeil/gjh_asl_json

.. image:: https://codecov.io/gh/ghackebeil/gjh_asl_json/branch/master/graph/badge.svg
    :target: https://codecov.io/gh/ghackebeil/gjh_asl_json

A simple tool providing similar functionality to that of the gjh
"solver" distributed with the AMPL Solver Library. NLP information
is summarized in a JSON-formatted output file.

Installation
~~~~~~~~~~~~

1. $ cd Thirdparty/
2. $ ./get.ASL
3. $ cd ..
4. $ make

Usage
~~~~~

1. $ gjh_asl_json stub.nl rows=stub.row cols=stub.col
2. $ python

.. code-block:: pycon

    >>> import json
    >>> with open('stub.json') as f:
    >>>    gjh = json.load(f)
