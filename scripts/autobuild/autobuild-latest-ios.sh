#!/bin/sh

CONFIG=$1
if [ "$CONFIG" == "" ]; then CONFIG=all; fi

# Source environment configuration.
source ../config.sh

# Build Traktor
pushd $TRAKTOR_HOME/build/ios
make -j 8 -f "Extern iOS.mak" $CONFIG
make -j 8 -f "Traktor iOS.mak" $CONFIG
popd
