#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Ensure correct path and build solution files.
pushd $TRAKTOR_HOME

CONFIG=$1
if [ -z $CONFIG ] ; then
	CONFIG="both"
fi

if [ $CONFIG == "static" ] ; then
	$SOLUTIONBUILDER -f=xcode -xcode-root-suffix=-static $TRAKTOR_HOME/resources/build/TraktorOSX.xms -d=DebugStatic -r=ReleaseStatic
elif [ $CONFIG == "shared" ] ; then
	$SOLUTIONBUILDER -f=xcode -xcode-root-suffix=-shared $TRAKTOR_HOME/resources/build/TraktorOSX.xms -d=DebugShared -r=ReleaseShared
else
	$SOLUTIONBUILDER -f=xcode -xcode-root-suffix=-static $TRAKTOR_HOME/resources/build/TraktorOSX.xms -d=DebugStatic -r=ReleaseStatic
	$SOLUTIONBUILDER -f=xcode -xcode-root-suffix=-shared $TRAKTOR_HOME/resources/build/TraktorOSX.xms -d=DebugShared -r=ReleaseShared
fi

popd

