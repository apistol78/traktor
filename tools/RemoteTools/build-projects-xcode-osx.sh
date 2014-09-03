#!/bin/sh

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../config.sh"

$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -xcode-root-suffix=-static RemoteToolsOSX.xms -d=DebugStatic -r=ReleaseStatic
