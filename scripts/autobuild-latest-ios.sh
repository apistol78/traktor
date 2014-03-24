#!/bin/sh

source ../config.sh

# Get normalized path to this script, excluding file name.
BUILD_LOG_DIR="`dirname \"$BASH_SOURCE\"`"
BUILD_LOG_DIR="`(cd \"$BUILD_LOG_DIR\" && pwd)`"

# Build Traktor
pushd $TRAKTOR_HOME

/bin/sh build-projects-make-ios.sh

pushd build/ios-i386
make ReleaseStatic 2>$BUILD_LOG_DIR/build-stderr.log
popd
pushd build/ios-x86_64
make ReleaseStatic 2>>$BUILD_LOG_DIR/build-stderr.log
popd
pushd build/ios-armv7
make ReleaseStatic 2>>$BUILD_LOG_DIR/build-stderr.log
popd
pushd build/ios-armv7s
make ReleaseStatic 2>>$BUILD_LOG_DIR/build-stderr.log
popd
pushd build/ios-arm64
make ReleaseStatic 2>>$BUILD_LOG_DIR/build-stderr.log
popd

popd

# Put built binaries into place
/bin/sh copy-latest-ios.sh

