#!/bin/sh

source ../config.sh

# Get normalized path to this script, excluding file name.
BUILD_LOG_DIR="`dirname \"$BASH_SOURCE\"`"
BUILD_LOG_DIR="`(cd \"$BUILD_LOG_DIR\" && pwd)`"

# Build Traktor
pushd $TRAKTOR_HOME

source build-projects-make-ios.sh

pushd build/ios-i386
make -j 8 ReleaseStatic 2>$BUILD_LOG_DIR/build-ios-release-stderr.log
make -j 8 DebugStatic 2>$BUILD_LOG_DIR/build-ios-debug-stderr.log
popd
#pushd build/ios-x86_64
#make -j 8 ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-release-stderr.log
#popd
pushd build/ios-armv7
make -j 8 ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-release-stderr.log
make -j 8 DebugStatic 2>$BUILD_LOG_DIR/build-ios-debug-stderr.log
popd
#pushd build/ios-armv7s
#make -j 8 ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-release-stderr.log
#popd
#pushd build/ios-arm64
#make -j 8 ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-release-stderr.log
#popd

popd

# Put built binaries into place
source copy-latest-ios.sh

