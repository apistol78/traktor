#!/bin/sh

source ../config.sh

# Get normalized path to this script, excluding file name.
BUILD_LOG_DIR="`dirname \"$BASH_SOURCE\"`"
BUILD_LOG_DIR="`(cd \"$BUILD_LOG_DIR\" && pwd)`"

# Reset log file and add time stamp.
echo "========== Build Begun ==========" >$BUILD_LOG_DIR/build-android-stderr.log
echo $(date +"%D %T") >>$BUILD_LOG_DIR/build-android-stderr.log

# Build Traktor
pushd $TRAKTOR_HOME
/bin/sh build-projects-make-android.sh

pushd build/android
echo "========== ReleaseStatic ==========" >>$BUILD_LOG_DIR/build-android-stderr.log
make -j 8 ReleaseStatic 2>>$BUILD_LOG_DIR/build-android-stderr.log
echo "========== DebugStatic ==========" >>$BUILD_LOG_DIR/build-android-stderr.log
make -j 8 DebugStatic 2>>$BUILD_LOG_DIR/build-android-stderr.log
popd

popd
