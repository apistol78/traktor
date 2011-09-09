#!/bin/sh
export TRAKTOR_HOME=$PWD
wine $TRAKTOR_HOME/bin/SolutionBuilder -f=xcode TraktorMacOSX.xms -d=DebugShared -r=ReleaseShared
