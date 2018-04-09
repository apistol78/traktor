#!/bin/sh

CONFIG=$1
if [ "$CONFIG" == "" ]; then CONFIG=all; fi

# Source environment configuration.
source ../config.sh

# Build Traktor
pushd $TRAKTOR_HOME/build/ios
make -f "Extern iOS.mak" $CONFIG
make -f "Traktor iOS.mak" $CONFIG
popd
