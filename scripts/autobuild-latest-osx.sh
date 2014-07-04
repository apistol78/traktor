#!/bin/sh

source ../config.sh

# Get normalized path to this script, excluding file name.
BUILD_LOG_DIR="`dirname \"$BASH_SOURCE\"`"
BUILD_LOG_DIR="`(cd \"$BUILD_LOG_DIR\" && pwd)`"

# Build Traktor
pushd $TRAKTOR_HOME
/bin/sh build-projects-make-osx.sh

pushd build/osx
make -j 8 ReleaseShared 2>$BUILD_LOG_DIR/build-osx-releaseshared-stderr.log
make -j 8 DebugShared 2>>$BUILD_LOG_DIR/build-osx-debugshared-stderr.log
make -j 8 ReleaseStatic 2>$BUILD_LOG_DIR/build-osx-releasestatic-stderr.log
popd

popd

# Put built binaries into place
/bin/sh copy-latest-osx.sh

