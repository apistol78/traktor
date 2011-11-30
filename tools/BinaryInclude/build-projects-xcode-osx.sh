#!/bin/sh

$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -xcode-root-suffix=-static BinaryIncludeMacOSX.xms -d=DebugStatic -r=ReleaseStatic
