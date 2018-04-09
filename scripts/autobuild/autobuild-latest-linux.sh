#!/bin/bash

CONFIG=$1
if [ "$CONFIG" == "" ]; then CONFIG=all; fi

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

# Build Traktor
pushd $TRAKTOR_HOME/build/linux
make -f "Extern Linux.mak" $CONFIG
make -f "Traktor Linux.mak" $CONFIG
popd
