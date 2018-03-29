#!/bin/bash

CONFIG=$1
if [ "$CONFIG" == "" ]; then CONFIG=all; fi

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

# Build Traktor
pushd $TRAKTOR_HOME/build/android
make -j 4 -f "Extern Android.mak" $CONFIG
make -j 4 -f "Traktor Android.mak" $CONFIG
popd
