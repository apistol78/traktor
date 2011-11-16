#!/bin/sh

export TRAKTOR_HOME=$PWD

wine $TRAKTOR_HOME/bin/SolutionBuilder -f=xcode -xcode-root-suffix=-static -i=ipad TraktorIPhone.xms -d=DebugStatic -r=ReleaseStatic
