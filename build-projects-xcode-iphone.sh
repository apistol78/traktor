#!/bin/sh

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Ensure correct path and build solution files.
pushd $TRAKTOR_HOME
$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -xcode-root-suffix=-static -i=ipad TraktorIPhone.xms -d=DebugStatic -r=ReleaseStatic
popd

