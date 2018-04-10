#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

# Build Traktor
pushd $TRAKTOR_HOME
rm -rf build/linux
rm -rf bin/latest/linux
popd
