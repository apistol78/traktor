#!/bin/bash

CONFIG=$1
if [ "$CONFIG" == "" ]; then CONFIG=all; fi

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

# Build Traktor
pushd $TRAKTOR_HOME/build/linux
make -j 4 -f "Extern Linux.mak" $CONFIG
make -j 4 -f "Traktor Linux.mak" $CONFIG
popd