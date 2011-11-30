#!/bin/sh

export TRAKTOR_HOME=$PWD

$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -xcode-root-suffix=-static -i=ipad TraktorIPhone.xms -d=DebugStatic -r=ReleaseStatic
