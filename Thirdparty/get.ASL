#!/bin/sh

set -e

rm -f solvers.tgz
wget http://www.ampl.com/netlib/ampl/solvers.tgz \
  || wget http://www.ampl.com/netlib/solvers.tgz
rm -rf solvers
tar xf solvers.tgz
rm solvers.tgz
cd solvers
sed -e 's/CFLAGS = /CFLAGS = -O3 -pipe -DNDEBUG -DASL_BUILD  -fPIC -DPIC /g' makefile.u > Makefile
cd ..
