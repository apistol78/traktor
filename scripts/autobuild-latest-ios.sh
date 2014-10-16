#!/bin/sh

source ../config.sh

CONFIG=$1
if [ -z $CONFIG ] ; then
	CONFIG="both"
fi

# Get normalized path to this script, excluding file name.
BUILD_LOG_DIR="`dirname \"$BASH_SOURCE\"`"
BUILD_LOG_DIR="`(cd \"$BUILD_LOG_DIR\" && pwd)`"

# Reset log file and add time stamp.
echo "========== Build Begun ($CONFIG) ==========" >$BUILD_LOG_DIR/build-ios-stderr.log
echo $(date +"%D %T") >>$BUILD_LOG_DIR/build-ios-stderr.log

# Build Traktor
pushd $TRAKTOR_HOME
source build-projects-make-ios.sh

pushd build/ios-i386
if [ $CONFIG == "both" ] || [ $CONFIG == "release" ] ; then
	echo "========== ReleaseStatic (i386) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	make ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
fi
if [ $CONFIG == "both" ] || [ $CONFIG == "debug" ] ; then
	echo "========== DebugStatic (i386) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	make DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
fi
popd

#pushd build/ios-x86_64
#if [ $CONFIG == "both" ] || [ $CONFIG == "release" ] ; then
	#echo "========== ReleaseStatic (x86_64) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	#make ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
#fi
#if [ $CONFIG == "both" ] || [ $CONFIG == "debug" ] ; then
	#echo "========== DebugStatic (x86_64) ==========" 2>>$BUILD_LOG_DIR/build-ios-stderr.log
	#make DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
#fi
#popd

pushd build/ios-armv7
if [ $CONFIG == "both" ] || [ $CONFIG == "release" ] ; then
	echo "========== ReleaseStatic (ARMv7) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	make ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
fi
if [ $CONFIG == "both" ] || [ $CONFIG == "debug" ] ; then
	echo "========== DebugStatic (ARMv7) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	make DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
fi
popd

pushd build/ios-armv7s
if [ $CONFIG == "both" ] || [ $CONFIG == "release" ] ; then
	echo "========== ReleaseStatic (ARMv7s) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	make ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
fi
if [ $CONFIG == "both" ] || [ $CONFIG == "debug" ] ; then
	echo "========== DebugStatic (ARMv7s) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	make DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
fi
popd

#pushd build/ios-arm64
#if [ $CONFIG == "both" ] || [ $CONFIG == "release" ] ; then
	#echo "========== ReleaseStatic (ARM64) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	#make ReleaseStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
#fi
#if [ $CONFIG == "both" ] || [ $CONFIG == "debug" ] ; then
	#echo "========== DebugStatic (ARM64) ==========" >>$BUILD_LOG_DIR/build-ios-stderr.log
	#make DebugStatic 2>>$BUILD_LOG_DIR/build-ios-stderr.log
#fi
#popd

popd

# Put built binaries into place
source copy-latest-ios.sh $CONFIG


