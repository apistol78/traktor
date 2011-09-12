#!/bin/sh

export TRAKTOR_HOME=$PWD

CONFIG=$1
if [ -z $CONFIG ] ; then
	CONFIG="both"
fi

if [ $CONFIG == "static" ] ; then
	wine $TRAKTOR_HOME/bin/SolutionBuilder -f=xcode -xcode-root-suffix=-static TraktorMacOSX.xms -d=DebugStatic -r=ReleaseStatic
elif [ $CONFIG == "shared" ] ; then
	wine $TRAKTOR_HOME/bin/SolutionBuilder -f=xcode -xcode-root-suffix=-shared TraktorMacOSX.xms -d=DebugShared -r=ReleaseShared
else
	wine $TRAKTOR_HOME/bin/SolutionBuilder -f=xcode -xcode-root-suffix=-static TraktorMacOSX.xms -d=DebugStatic -r=ReleaseStatic
	wine $TRAKTOR_HOME/bin/SolutionBuilder -f=xcode -xcode-root-suffix=-shared TraktorMacOSX.xms -d=DebugShared -r=ReleaseShared
fi

