#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

# Get normalized path to this script, excluding file name.
BUILD_LOG_DIR="`dirname \"$BASH_SOURCE\"`"
BUILD_LOG_DIR="`(cd \"$BUILD_LOG_DIR\" && pwd)`"

# Reset log file and add time stamp.
echo "========== Build Begun ==========" >$BUILD_LOG_DIR/build-linux-stderr.log
echo $(date +"%D %T") >>$BUILD_LOG_DIR/build-linux-stderr.log

# Build Traktor
pushd $TRAKTOR_HOME
/bin/bash build-projects-make-linux.sh
popd

pushd $TRAKTOR_HOME/build/linux
echo "========== DebugShared ==========" >>$BUILD_LOG_DIR/build-linux-stderr.log
make -j 4 DebugShared 2>>$BUILD_LOG_DIR/build-linux-stderr.log
echo "========== ReleaseShared ==========" >>$BUILD_LOG_DIR/build-linux-stderr.log
make -j 4 ReleaseShared 2>>$BUILD_LOG_DIR/build-linux-stderr.log
popd
