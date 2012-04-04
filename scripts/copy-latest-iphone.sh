#!/bin/sh

source ../config.sh

# Copy products into bin directory.
pushd $TRAKTOR_HOME

mkdir -p bin/latest/iphone-simulator/releasestatic
mkdir -p bin/latest/iphone/releasestatic

# Copy simulator binaries.
pushd bin/latest/iphone-simulator/releasestatic

TRAKTOR_BUILD=$TRAKTOR_HOME/build/iphone-static/build/Release-iphonesimulator

# Copy archives.
cp -Rf $TRAKTOR_BUILD/*.a .

popd

# Copy native binaries.
pushd bin/latest/iphone/releasestatic

TRAKTOR_BUILD=$TRAKTOR_HOME/build/iphone-static/build/Release-iphoneos

# Copy archives.
cp -Rf $TRAKTOR_BUILD/*.a .

popd

popd
