#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Build
pushd $TRAKTOR_HOME/build/linux
make -j6 -f 'Extern Linux.mak' $1
make -j6 -f 'Traktor Linux.mak' $1
popd
