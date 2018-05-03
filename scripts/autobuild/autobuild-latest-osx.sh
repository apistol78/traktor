#!/bin/sh

CONFIG=$1
if [ "$CONFIG" == "" ]; then CONFIG=all; fi

# Source environment configuration.
source ../config.sh

# Build Traktor
pushd $TRAKTOR_HOME/build/osx

make -f "Extern OSX.mak" $CONFIG
if ! [ $? -eq 0 ]; then exit 1; fi

make -f "Traktor OSX.mak" $CONFIG
if ! [ $? -eq 0 ]; then exit 1; fi

popd
