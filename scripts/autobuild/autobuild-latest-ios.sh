#!/bin/sh

CONFIG=$1
if [ "$CONFIG" == "" ]; then CONFIG=all; fi

# Source environment configuration.
source ../config.sh

# Get normalized path to this script, excluding file name.
BUILD_LOG_DIR="`dirname \"$BASH_SOURCE\"`"
BUILD_LOG_DIR="`(cd \"$BUILD_LOG_DIR\" && pwd)`"

# Reset log file and add time stamp.
echo "========== Build Begun ($CONFIG) ==========" >$BUILD_LOG_DIR/build-ios-stderr.log
echo $(date +"%D %T") >>$BUILD_LOG_DIR/build-ios-stderr.log

# Build Traktor
pushd $TRAKTOR_HOME/build/ios
if [ $CONFIG == "both" ] || [ $CONFIG == "release" ] ; then
	echo "========== ReleaseStatic ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	make -j 4 ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
fi
if [ $CONFIG == "both" ] || [ $CONFIG == "debug" ] ; then
	echo "========== DebugStatic ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	make -j 4 DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
fi
popd
