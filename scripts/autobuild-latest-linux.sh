#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

# Build Traktor
pushd $TRAKTOR_HOME
/bin/bash build-projects-make-linux.sh
popd

pushd $TRAKTOR_HOME/build/linux
make -j 8 DebugShared 2>&1 | tee build.log
make -j 8 ReleaseShared 2>&1 | tee build.log
popd

# Put built binaries into place
/bin/bash copy-latest-linux.sh

