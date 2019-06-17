#!/bin/bash

CONFIG=$1
if [ "$CONFIG" == "" ]; then CONFIG=all; fi

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

# Build Traktor
pushd $TRAKTOR_HOME/build/rpi

make -f "Extern Raspberry Pi.mak" $CONFIG
if ! [ $? -eq 0 ]; then exit 1; fi

make -f "Traktor Raspberry Pi.mak" $CONFIG
if ! [ $? -eq 0 ]; then exit 1; fi

popd
