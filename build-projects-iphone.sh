#!/bin/sh

mkdir $TRAKTOR_HOME/build

$TRAKTOR_HOME/bin/MacOSX/SolutionBuilder -f=make $TRAKTOR_HOME/TraktorIPhone.xms -d=gnu -p=osx -c=$TRAKTOR_HOME/bin/make-config-iphone.inc
