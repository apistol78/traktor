#!/bin/sh

$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -xcode-root-suffix=-static RemoteToolsMacOSX.xms -d=DebugStatic -r=ReleaseStatic
