#!/bin/sh

source ../config.sh

# Get normalized path to this script, excluding file name.
BUILD_LOG_DIR="`dirname \"$BASH_SOURCE\"`"
BUILD_LOG_DIR="`(cd \"$BUILD_LOG_DIR\" && pwd)`"

# Reset log file and add time stamp.
echo "========== Build Begun ==========" >$BUILD_LOG_DIR/build-ios-stderr.log
echo $(date +"%D %T") >>$BUILD_LOG_DIR/build-ios-stderr.log

# Build Traktor
pushd $TRAKTOR_HOME
source build-projects-make-ios.sh

pushd build/ios-i386
echo "========== ReleaseStatic (i386) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
make ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
echo "========== DebugStatic (i386) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
make DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
popd

#pushd build/ios-x86_64
#echo "========== ReleaseStatic (x86_64) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
#make ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
#echo "========== DebugStatic (x86_64) ==========" 2>>$BUILD_LOG_DIR/build-ios-stderr.log
#make DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
#popd

pushd build/ios-armv7
echo "========== ReleaseStatic (ARMv7) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
make ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
echo "========== DebugStatic (ARMv7) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
make DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
popd

pushd build/ios-armv7s
echo "========== ReleaseStatic (ARMv7s) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
make ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
echo "========== DebugStatic (ARMv7s) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
make DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
popd

#pushd build/ios-arm64
#echo "========== ReleaseStatic (ARM64) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
#make ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
#echo "========== DebugStatic (ARM64) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
#make DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
#popd

popd

# Put built binaries into place
source copy-latest-ios.sh

