#!/bin/sh
export TRAKTOR_HOME=$PWD

if [ $1 -eq "static" ]; then
	wine $TRAKTOR_HOME/bin/SolutionBuilder -f=xcode -rootPath=$(TRAKTOR_HOME)/build/macosx-static TraktorMacOSX.xms -d=DebugStatic -r=ReleaseStatic
else
	wine $TRAKTOR_HOME/bin/SolutionBuilder -f=xcode -rootPath=$(TRAKTOR_HOME)/build/macosx-shared TraktorMacOSX.xms -d=DebugShared -r=ReleaseShared
fi
