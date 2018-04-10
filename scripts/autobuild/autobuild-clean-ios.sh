#!/bin/sh

# Source environment configuration.
source ../config.sh

# Build Traktor
pushd $TRAKTOR_HOME
rm -rf build/ios
rm -rf bin/latest/ios
popd
