#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

# Copy debug products into bin directory.
pushd $TRAKTOR_HOME

mkdir -p bin/latest/linux/debugshared
cd bin/latest/linux/debugshared

# Copy binaries.
cp -f "$TRAKTOR_HOME/build/linux/debugshared/*" .
if [ $(uname -m) == 'x86_64' ]; then
	cp -f "$STEAMWORKS_SDK/redistributable_bin/linux64/*" .
	cp -f "$FBX_SDK/lib/gcc4/x64/*.so" .
else
	cp -f "$STEAMWORKS_SDK/redistributable_bin/linux32/*" .
	cp -f "$FBX_SDK/lib/gcc4/x86/*.so" .
fi

popd

# Copy release products into bin directory.
pushd $TRAKTOR_HOME

mkdir -p bin/latest/linux/releaseshared
cd bin/latest/linux/releaseshared

# Copy binaries.
cp -f "$TRAKTOR_HOME/build/linux/releaseshared/*" .
if [ $(uname -m) == 'x86_64' ]; then
	cp -f "$STEAMWORKS_SDK/redistributable_bin/linux64/*" .
	cp -f "$FBX_SDK/lib/gcc4/x64/*.so" .
else
	cp -f "$STEAMWORKS_SDK/redistributable_bin/linux32/*" .
	cp -f "$FBX_SDK/lib/gcc4/x86/*.so" .
fi

popd
