#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

# Build Traktor
#---------------
pushd $TRAKTOR_HOME
/bin/bash build-projects-cblocks-linux.sh
popd

pushd $TRAKTOR_HOME/build/linux
codeblocks --build --target=ReleaseShared "Traktor Linux.workspace"
popd

# Put built binaries into place
#-------------------------------
/bin/bash copy-latest-linux.sh

