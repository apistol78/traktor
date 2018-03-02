#!/bin/sh

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Ensure correct path and build solution files.
pushd $TRAKTOR_HOME
$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -i TraktorIOS.xms -d=DebugStatic -r=ReleaseStatic
popd

