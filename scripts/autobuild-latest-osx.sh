#!/bin/sh

source ../config.sh

# Build Traktor
pushd $TRAKTOR_HOME
/bin/sh build-projects-make-osx.sh

pushd build/osx
make ReleaseShared
popd

popd

# Put built binaries into place
/bin/sh copy-latest-osx.sh

