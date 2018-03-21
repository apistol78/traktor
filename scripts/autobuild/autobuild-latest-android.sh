#!/bin/sh

CONFIG=$1
if [ "$CONFIG" == "" ]; then CONFIG=all; fi

# Source environment configuration.
source ../config.sh

# Build Traktor
pushd $TRAKTOR_HOME/build/android
make -j 8 -f "Extern Android.mak" $CONFIG
make -j 8 -f "Traktor Android.mak" $CONFIG
popd
