#!/bin/sh

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Ensure correct path and build solution files.
pushd $TRAKTOR_HOME

CONFIG=$1
if [ -z $CONFIG ] ; then
	CONFIG="both"
fi

if [ $CONFIG == "static" ] ; then
	$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -xcode-root-suffix=-static TraktorOSX.xms -d=DebugStatic -r=ReleaseStatic
elif [ $CONFIG == "shared" ] ; then
	$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -xcode-root-suffix=-shared TraktorOSX.xms -d=DebugShared -r=ReleaseShared
else
	$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -xcode-root-suffix=-static TraktorOSX.xms -d=DebugStatic -r=ReleaseStatic
	$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -xcode-root-suffix=-shared TraktorOSX.xms -d=DebugShared -r=ReleaseShared
fi

popd

