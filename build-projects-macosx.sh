#!/bin/sh

mkdir $TRAKTOR_HOME/build

$TRAKTOR_HOME/bin/MacOSX/SolutionBuilder -f=make $TRAKTOR_HOME/TraktorMacOSX.xms -d=gnu -p=osx
