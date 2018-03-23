#!/bin/sh

CONFIG=$1
if [ "$CONFIG" == "" ]; then CONFIG=all; fi

# Source environment configuration.
source ../config.sh

# Build Traktor
pushd $TRAKTOR_HOME/build/osx
make -j 4 -f "Extern OSX.mak" $CONFIG
make -j 4 -f "Traktor OSX.mak" $CONFIG
popd
