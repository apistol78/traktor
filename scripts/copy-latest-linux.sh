#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

# Copy products into bin directory.
pushd $TRAKTOR_HOME

mkdir -p bin/latest/linux/releaseshared
cd bin/latest/linux/releaseshared

# Copy binaries.
cp -f $TRAKTOR_HOME/build/linux/releaseshared/* .

popd
