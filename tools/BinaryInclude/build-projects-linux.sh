#!/bin/sh

mkdir build

$TRAKTOR_HOME/bin/Linux/SolutionBuilder -f=make BinaryIncludeLinux.xms -d=gnu -p=linux
