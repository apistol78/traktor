#!/bin/sh

mkdir $TRAKTOR_HOME/build

$TRAKTOR_HOME/bin/Linux/SolutionBuilder -f=make $TRAKTOR_HOME/TraktorLinux.xms -d=gnu -p=linux

