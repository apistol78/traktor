#!/bin/sh

# Source environment configuration.
source ../config.sh

# Build Traktor
pushd $TRAKTOR_HOME
rm -rf build/osx
rm -rf bin/latest/osx
popd
