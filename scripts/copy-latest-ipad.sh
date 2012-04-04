#!/bin/sh

source ../config.sh

# Copy products into bin directory.
pushd $TRAKTOR_HOME

mkdir -p bin/latest/ipad-simulator/releasestatic
mkdir -p bin/latest/ipad/releasestatic

# Copy simulator binaries.
pushd bin/latest/ipad-simulator/releasestatic

TRAKTOR_BUILD=$TRAKTOR_HOME/build/ipad-static/build/Release-iphonesimulator

# Copy archives.
cp -Rf $TRAKTOR_BUILD/*.a .

popd

# Copy native binaries.
pushd bin/latest/ipad/releasestatic

TRAKTOR_BUILD=$TRAKTOR_HOME/build/ipad-static/build/Release-iphoneos

# Copy archives.
cp -Rf $TRAKTOR_BUILD/*.a .

popd

popd
