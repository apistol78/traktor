#!/bin/sh

source ../config.sh

# Copy products into bin directory.
pushd $TRAKTOR_HOME

mkdir -p bin/latest/pnacl/releasestatic
cd bin/latest/pnacl/releasestatic

TRAKTOR_BUILD=$TRAKTOR_HOME/build/pnacl/releasestatic

# Copy binaries.
cp -f $TRAKTOR_BUILD/*.a .

popd
